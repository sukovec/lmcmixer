function Fader(name) {
	var fadernode = this.tmpl.cloneNode();
	var info = {};
	this.traversenode(fadernode, info);

	if (info.fader_name)
		info.fader_name.innerHTML = name;


	this.container.appendChild(fadernode);
}

Fader.prototype.traversenode = function(node, info) {
	for (var i = 0; i < node.childNodes.length; i++) {
		//search for classes: fader_name, volume, panorama

		var cnode = node.childNodes[i];
		if (cnode.nodeType == Node.ELEMENT_NODE) {
			if (cnode.classList.contains("fader_name"))
				info.fader_name = cnode;
			else if (cnode.classList.contains("volume"))
				info.volume = cnode;
			else if (cnode.classList.contains("panorama"))
				info.panorama = cnode;
			else 
				this.traversenode(cnode, info);
		}
	}
}

function InitFaderTemplate() {
	var tmpl = document.getElementById("template");
	var faders = document.getElementById("faders");

	for(var i = 0; i < tmpl.childNodes.length; i++) {
		if (tmpl.childNodes[i].nodeType == Node.ELEMENT_NODE && 
				tmpl.childNodes[i].classList.contains("fader")) {
			tmpl = tmpl.childNodes[i];
			break;
		}
	}

	// traverse it

	Fader.prototype.tmpl = tmpl;
	Fader.prototype.container = faders;
};
