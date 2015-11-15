function State(msg) {
	var st = document.getElementById("status");
	if (!st) return;

	st.innerHTML = msg;
}

function run() {
	InitFaderTemplate();
	new LMCMixerWS();
}
