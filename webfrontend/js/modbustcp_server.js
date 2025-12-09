class ModbusTCP_server{
	constructor(jsonRPC, auth, cloud, lang){

		this.jsonRPC = jsonRPC;
		this.auth = auth;
		this.cloud = cloud;
		this.lang = lang;
	}

	run() {
		var that = this;

		$('#spacer_modbustcp_s').load('html/modbustcp_server.html #content', function(){
			that.setLanguage(that.lang);
			//callback for button click
			$('#btn_modbustcp_s_save').click(function(){
				that.saveConfig();
			});

			//enable/disable inputs if modbustcp_s was activated/deactivated
			$('#input_modbustcp_s_enable').change(function(){
				if ($('#input_modbustcp_s_enable').is(':checked') == true){
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
			$('#spacer_modbustcp_s').show();
		}else{
			$('#spacer_modbustcp_s').hide();
		}
	}

	/**
	 * Extern called function which prevent to edit fields after logout.
	 */
	disableSet(){
		//AZURE
		$('#input_modbustcp_s_enable').attr('disabled', true);
		$('#btn_modbustcp_s_save').attr('disabled', true);
		this.disableConfig();
	}

	/**
	 * Extern called function which allows to edit fields after login.
	 */
	enableSet(){
		//AZURE
		$('#input_modbustcp_s_enable').attr('disabled', false);
		$('#btn_modbustcp_s_save').attr('disabled', false);
		var that = this;
		if ($('#input_modbustcp_s_enable').is(':checked') == true){
			that.enableConfig();
		}
	}

	/**
	 * Set All Inputs to read only and restore default configuration
	 */
	disableConfig(){
		$('#input_modbustcp_s_pcs').attr('readonly', true);
	}

	/**
	 * Remove all readonly tags from inputs.
	 */
	enableConfig(){
		$('#input_modbustcp_s_pcs').attr('readonly', false);
	}

	/**
	 * Reading all parameters form input and save these to the chip.
	 */
	saveConfig() {

		var params = {};
		params["active"] = $('#input_modbustcp_s_enable').is(':checked');
		params["auth"] = this.auth.getCredentials();
		params["uid"] = parseInt($('#input_modbustcp_s_clientid').val());
		params["port"] = parseInt($('#input_modbustcp_s_port').val());

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

		if(result["active"]){
			this.enableConfig();
		}else{
			this.disableConfig();
		}

		$('#input_modbustcp_s_enable').prop( "checked", result["active"] );
		$('#input_modbustcp_s_status').val(this.getStatusText(result));

		$('#input_modbustcp_s_clientid').val(result["uid"].toString());

		$('#input_modbustcp_s_port').val(result["port"].toString());

	}

	/**
	 * Reading connection state from chip an show these in an readonly input.
	 */
	getStatus(result) {
		$('#input_modbustcp_s_status').val(this.getStatusText(result));
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