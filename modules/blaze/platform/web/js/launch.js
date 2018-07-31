Module['locateFile'] = (url) => {
	return "/static/" + url;
}

Module['print'] = (() => {
	let element = document.getElementById('output');
	if (element) {
		element.value = '';
	}

	return (...text) => {
		text = text.slice().join(' ');

		console.log(text);

		if (element) {
			element.value += text + '\n';
			element.scrollTop = element.scrollHeight;
		}
	};
})();

Module['preRun'] = [
	() => {
		FS.mkdir('/data');

		// Preload all archives, game will start after everything is loaded
		if (typeof archives !== 'undefined') {
			archives.forEach( (archive) => {
				FS.createPreloadedFile('/data', archive, Module.locateFile(archive), true, false);
			});
		}
	}
];

function getLog() {
	return new TextDecoder('utf-8').decode(FS.readFile('/data/blaze.log'));
}
