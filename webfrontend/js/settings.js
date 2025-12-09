class Settings{
	constructor(jsonRPC, auth, lang){

		this.lang = lang;
		this.jsonRPC = jsonRPC;
		this.auth = auth;
		this.updateInterval = 10; //every 10 seconds by default

	}
	run() {
		var that = this;
		//Initialize Device Infos
		$('#system_extension_field').load('html/settings.html #content', function(){
			that.setLanguage(that.lang);

			that.refresh();
			that.getData();


			//callback for button click
			$('#btn_data_save').click(function(){
				that.saveConfig();
			});

		});
	}

	setLanguage(lang){
		this.lang=lang;
		if(lang == "de"){
			$('.lang-de').show();
			$('.lang-en').hide();

			$('#data_displayUnits_met').text("metrisch");
			$('#data_displayUnits_imp').text("imperial");
		
		}else{
			$('.lang-de').hide();
			$('.lang-en').show();

			$('#data_displayUnits_met').text("metric");
			$('#data_displayUnits_imp').text("imperial");
		}
	}

	refresh(){
		this.getConfig();
	}

	getData() {
		var that = this;

		that.jsonRPC.call('extension.get', { 'select': 'config' }).then(function (result) {

		});
	}

	saveConfig() {

		var params = {};
		// add more params
		
		//add user data
		params["auth"] = this.auth.getCredentials();

		var that = this;

		//call rpc
		that.jsonRPC.call('extension.set', params).then(function (result) {
			//parse result
			if(result["state"] == 0) {
				$('#modal_success_text').html("Your changes have been saved!");
				$('#modal_success').modal('show');
				setTimeout(function(){
					$('#modal_success').modal('hide');
				}, 1000);
			}else{
				$('#modal_error_text').html("Your changes have not been saved!");
				$('#modal_error').modal('show');
				setTimeout(function(){
					$('#modal_error').modal('hide');
				}, 1000);

				that.auth.handleError(result["state"]);
			}
		});

		that.refresh();

	}

	getConfig() {
		var that = this;

		that.jsonRPC.call('extension.get', {'select':'config'}).then(function (result) {
	
			// do something with results

		});
	}

	celciusToFarrenheit(x){
		if(x == "") return "";
		return ((x * 9/5) + 32).toFixed(1);
	}

	farrenheitToCelcius(x){
		if(x == "") return "";
		return ((x - 32) * 5/9).toFixed(1);
	}

	disableSet(){
		$('.form_data').attr('disabled', true);
	}

	enableSet(){
		$('.form_data').attr('disabled', false);
	}
}