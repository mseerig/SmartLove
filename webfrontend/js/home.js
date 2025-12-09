/*$.getScript( "js/dashboard.js");
$.getScript( "js/network.js");
$.getScript( "js/mqtt.js");
$.getScript( "js/update.js");*/

class Home{
	constructor(jsonRPC, auth, lang){

		this.jsonRPC = jsonRPC;
		this.auth = auth;
		this.lang = lang;
		this.dash = new Dash(jsonRPC, auth, lang, this);
		this.graphs = new Graph(jsonRPC, auth, lang, this);
		this.cloud = new Cloud(jsonRPC, auth, lang);
		this.network = new Network(jsonRPC, auth, lang, this.cloud);
		this.settings = new Settings(jsonRPC, auth, lang);
		this.system = new System(jsonRPC, auth, lang, this.settings);
		this.events = new Events(jsonRPC, auth, lang);
		//this.about = new About(jsonRPC, auth, lang, this);
	}
	run() {
		var that = this;
		$('#home').load('html/home.html #content', function(){
			that.setLanguage(that.lang);
			that.dash.run();
			that.graphs.run();
			that.network.run();
			//that.cloud.run();
			//that.settings.run();
			that.events.run();
			that.system.run();
			//that.about.run();

			$('#tab_dash_btn').on("click", function(){
				that.showTab('dash');
			});
			$('#tab_graphs_btn').on("click", function(){
				that.showTab('graphs');
			});
			$('#tab_network_btn').on("click", function(){
				that.showTab('network');
			});
			$('#tab_cloud_btn').on("click", function(){
				that.showTab('cloud');
			});
			$('#tab_events_btn').on("click", function(){
				that.showTab('events');
			});
			$('#tab_system_btn').on("click", function(){
				that.showTab('system');
			});
			/*$('#tab_about_btn').on("click", function(){
				that.showTab('about');
			});*/

			setInterval(function(){ that.checkLoginState();}, 1000);
		});
	}

	refresh(){
		this.dash.refresh(); 
		this.network.refresh();
		this.cloud.refresh();
		this.events.refresh();
		this.system.refresh();
		//this.about.refresh();
	}

	showTab(tab){
		$('.tab_content').addClass("display-none");
		$('#tab_'+tab).removeClass("display-none");

		//reset highligting for tabs
		$('.nav-box').removeClass("nav-active");
		$('.nav-box').addClass("nav-inactive");
		//highlight the right tab
		$('#tab_'+tab+'_box').removeClass("nav-inactive");
		$('#tab_'+tab+'_box').addClass("nav-active");

		switch(tab){
			case 'dash': 
				
				this.dash.refresh(); 
				break;

			case 'network': 
				this.network.refresh();
				this.cloud.refresh();
				break;

			case 'events':
				this.events.refresh();

			case 'system':
				this.system.refresh();
				break;

			/*case 'about':
				this.about.refresh();
				break;*/
		}
	}

	setLanguage(lang){
		this.lang = lang;
		this.dash.setLanguage(lang);
		this.graphs.setLanguage(lang);
		this.network.setLanguage(lang);
		this.cloud.setLanguage(lang);
		this.events.setLanguage(lang);
		this.settings.setLanguage(lang);
		this.system.setLanguage(lang);
		//this.about.setLanguage(lang);
		if(lang == "de"){
			$('.lang-de').show();
			$('.lang-en').hide();
		}else{
			$('.lang-de').hide();
			$('.lang-en').show();
		}
	}

	checkLoginState(){
		if(this.auth.isLoggedIn()) {
			this.dash.enableSet();
			this.graphs.enableSet();
			this.network.enableSet();
			this.cloud.enableSet();
			this.events.enableSet()
			this.settings.enableSet();
			this.system.enableSet();
			//this.about.enableSet();
		} else {
			this.dash.disableSet();
			this.graphs.disableSet();
			this.network.disableSet();
			this.cloud.disableSet();
			this.events.disableSet();
			this.settings.disableSet();
			this.system.disableSet();
			//this.about.enableSet();
		}
	}
}