class Azure{
	constructor(jsonRPC, auth, cloud, lang){

		this.jsonRPC = jsonRPC;
		this.auth = auth;
		this.cloud = cloud;
		this.lang = lang;
	}

	run() {
		var that = this;

		$('#spacer_azure').load('html/azure.html #content', function(){
			that.setLanguage(that.lang);
			//callback for button click
			$('#btn_azure_save').click(function(){
				that.saveConfig();
			});

			//enable/disable inputs if azure was activated/deactivated
			$('#input_azure_enable').change(function(){
				if ($('#input_azure_enable').is(':checked') == true){
					that.enableConfig();
				}else{
					that.disableConfig();
				}
			});
		});
	}

	setLanguage(lang){
		this.lang=lang;
		if(lang == "de"){
			$('.lang-de').show();
			$('.lang-en').hide();
		}else{
			$('.lang-de').hide();
			$('.lang-en').show();
		}
	}

	setActive(active){
		if(active == true) {
			$('#spacer_azure').show();
		}else{
			$('#spacer_azure').hide();
		}
	}

	/**
	 * Extern called function which prevent to edit fields after logout.
	 */
	disableSet(){
		//AZURE
		$('#input_azure_enable').attr('disabled', true);
		$('#btn_azure_save').attr('disabled', true);
		this.disableConfig();
	}

	/**
	 * Extern called function which allows to edit fields after login.
	 */
	enableSet(){
		//AZURE
		$('#input_azure_enable').attr('disabled', false);
		$('#btn_azure_save').attr('disabled', false);
		var that = this;
		if ($('#input_azure_enable').is(':checked') == true){
			that.enableConfig();
		}
	}

	/**
	 * Set All Inputs to read only and restore default configuration
	 */
	disableConfig(){
		$('#input_azure_pcs').attr('readonly', true);
	}

	/**
	 * Remove all readonly tags from inputs.
	 */
	enableConfig(){
		$('#input_azure_pcs').attr('readonly', false);
	}

	/**
	 * Reading all parameters form input and save these to the chip.
	 */
	saveConfig() {

		var params = {};
		params["active"] = $('#input_azure_enable').is(':checked');
		if($('#input_azure_pcs').val() != ""){
			params["primaryConnectionString"] = $('#input_azure_pcs').val();
		}
		params["auth"] = this.auth.getCredentials();

		var that = this;
		this.jsonRPC.call('cloud.setConfig', params).then(function (result) {
			if(result["state"] == 0) {
				$('#modal_success_text_en').html("Your changes has been saved!");
				$('#modal_success_text_de').html("Die Einstellungen wurden gespeichert!");
				$('#modal_success').modal('show');
				setTimeout(function(){
					$('#modal_success').modal('hide');
				}, 1000);
			}else{
				$('#modal_error_text_en').html("Your changes has not been saved!");
				$('#modal_error_text_de').html("Die Einstellungen wurden nicht gespeichert!");
				$('#modal_error').modal('show');
				setTimeout(function(){
					$('#modal_error').modal('hide');
				}, 1000);

				that.auth.handleError(result["state"]);
			}
		});

		//Wait 1 seconds before checking state
		var that = this;
		setTimeout(function(){
			that.cloud.getConfig();
		}, 2000);
	}

	/**
	 * reading all configuration parameters from the chip in to the input fields.
	 */
	getConfig(result) {
		//console.log("AZURE getConfig");

		//console.log("registered");
		$('#form_azure_config').show();
		$('#form_azure_activation').hide();

		if(result["active"]){
			this.enableConfig();
		}else{
			this.disableConfig();
		}

		$('#input_azure_enable').prop( "checked", result["active"] );
		$('#input_azure_status').val(this.getStatusText(result));

		$('#input_azure_hostname').val(result["hostname"]);
		$('#input_azure_deviceId').val(result["deviceID"]);
		if(result["hostname"] != ""){
			$('#input_azure_pcs').val("xxxxxxxxxxxxxxxx");
		}else{
			$('#input_azure_pcs').val("");
		}
	}

	/**
	 * Reading connection state from chip an show these in an readonly input.
	 */
	getStatus(result) {
		$('#input_azure_status').val(this.getStatusText(result));
	}

	getStatusText(result) {
		if(this.lang == 'de'){
			if(result["state"] == 'connected'){
				return "verbunden";
			}else{
				return "nicht verbunden";
			}
		}else{
			if(result["state"] == 'connected'){
				return "Connected";
			}else{
				return "Not connected";
			}
		}
	}

}