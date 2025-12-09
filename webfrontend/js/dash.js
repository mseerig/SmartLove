class Dash {
	constructor(jsonRPC, auth, lang, that) {
		this.that = that;
		this.lang = lang;
		this.jsonRPC = jsonRPC;
		this.auth = auth;
	}
	
	run() {
		var that = this;
		//Initialize Device Infos
		$('#tab_dash').load('html/dash.html #content', function () {
			that.setLanguage(that.lang);

			$(".GaugeMeter").gaugeMeter();// Initialize GaugeMeter plugin

			that.refresh();
		});
	}

	setLanguage(lang) {
		$(".GaugeMeter").empty();
		this.lang = lang;
		if (lang == "de") {
			$('.lang-de').show();
			$('.lang-en').hide();
		} else {
			$('.lang-de').hide();
			$('.lang-en').show();
		}
		$(".GaugeMeter").gaugeMeter();
	}

	refresh() {
		var that = this;
		this.getConfig();
		this.getData();
		setTimeout(function(){
			that.refresh();
		}, 2000);

	}

	getConfig() {
		var that = this;

		that.jsonRPC.call('extension.get', { 'select': 'config' }).then(function (result) {

			// do something with "result"
		});
	}


	getData() {
		var that = this;

		that.jsonRPC.call('extension.get', { 'select': 'data' }).then(function (result) {

			// do something with "result"

		});
	}

	saveConfig() {

	}

	disableSet() {
		$('.form_dashboard').attr('disabled', true);
	}
	enableSet() {
		$('.form_dashboard').attr('disabled', false);
	}
}



