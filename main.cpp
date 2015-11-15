#include <alsa/asoundlib.h>
#include <alsa/mixer.h>
#include <signal.h>
#include <stdio.h>


#include "config.h"
#include "websock.h"

bool run;
void sig(int sg) {
	run = false;
}

const char * const names[] = {"Vox 1", "Gtr 1", "Vox 2", "Gtr 2", "Bass", "Kick", "Snare", "OH"};

int main(int argc, char ** argv) {
	signal(SIGINT, sig);
	

	WebsockFrontend wf;
	if (wf.Init()) {
		wf.SetInputNames(names);
		wf.Run();
	}
	else {
		printf("Cannot run WebSock frontend!\n");
	}

	run = true;
	while(run) {
		sleep(1);
	}

	wf.Stop();

	return 0;
	long min, max;
	snd_mixer_t *handle;
	snd_mixer_selem_id_t *sid;
	const char *card = ALSA_CARD;
	const char *selem_name = "Master";

	int volume = 100;

	snd_mixer_open(&handle, 0);
	snd_mixer_attach(handle, card);
	snd_mixer_selem_register(handle, NULL, NULL);
	snd_mixer_load(handle);

	snd_mixer_selem_id_alloca(&sid);
	snd_mixer_selem_id_set_index(sid, 0);
	snd_mixer_selem_id_set_name(sid, selem_name);
	snd_mixer_elem_t* elem = snd_mixer_find_selem(handle, sid);

	snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
	snd_mixer_selem_set_playback_volume_all(elem, volume * max / 100);

	snd_mixer_close(handle);
}
