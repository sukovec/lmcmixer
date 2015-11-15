function LMCMixerWS() {
	var connto = "ws://" + window.location.host + "/mixer";
	this.ws = new WebSocket(connto);
	this.ws.binaryType = "arraybuffer";
	var self = this;

	this.ws.onopen = function() {
		State("Connection opened, waiting for HELLO packet");
	}

	this.ws.onerror = function() {
		State("Connection error");
	}

	this.ws.onmessage = function(msg) {
		self.ParsePacket(msg.data);
	}

}


LMCMixerWS.prototype.ProcessHello = function(ds) {
	this.receivedhello = true;
	this.incount = ds.readInt8();
	this.outcount = ds.readInt8();

	this.inputs = [];
	for (var i = 0; i < this.incount; i++) {
		var len = ds.readInt8();
		if (len == 0) {
			for (var j = i; j < this.incount; j++)
				this.inputs.push(new Fader("Input " + j));
			break;
		}
		this.inputs.push(new Fader(ds.readString(len)));
	}
}
LMCMixerWS.prototype.ParsePacket = function(data) {
	if (!this.ds)
		this.ds = new DataStream(data);
	else {
		alert("unimplemented");
	}

	var head = this.ds.readString(4);
	if (head != "LMCM") { State("Wrong header"); return false; }
	var cmd = this.ds.readString(5);
	switch(cmd) {
		case "HELLO":
			this.ProcessHello(this.ds);
			break;
	}

}
