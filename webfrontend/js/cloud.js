class Cloud{
	constructor(jsonRPC, auth, lang){
		this.lang=lang;
		this.jsonRPC = jsonRPC;
		this.auth = auth;
		this.select = "none";
		this.mqtt = new Mqtt(jsonRPC, auth, this.lang);
		this.azure = new Azure(jsonRPC, auth, this, this.lang);
		this.modbustcp_s = new ModbusTCP_server(jsonRPC, auth, this, this.lang);
	}

	run() {
		var that = this;
		$('#spacer_cloud').load('html/cloud.html #content', function(){
			that.setLanguage(that.lang);
			that.mqtt.setActive(false);
			that.azure.setActive(false);
			that.modbustcp_s.setActive(false);
			that.mqtt.run();
			that.azure.run();
			that.modbustcp_s.run();

			$('#btn_cloud_select').click(function(){
				that.saveConfig();
			});

			that.refresh();
			that.getStatus();
		});
	}

	setLanguage(lang){
		this.lang=lang;
		if(lang == "de"){
			$('.lang-de').show();
			$('.lang-en').hide();

			$('#select_cloud_protocol_none').html('auswählen ...');
			$('#select_cloud_protocol_mqtt').html('klassisches MQTT');
			$('#select_cloud_protocol_azure').html('Microsoft Azure');
			$('#select_cloud_protocol_modbustcp_s').html('ModbusTCP Server');
		}else{
			$('.lang-de').hide();
			$('.lang-en').show();

			$('#select_cloud_protocol_none').html('Select ...');
			$('#select_cloud_protocol_mqtt').html('Generic MQTT');
			$('#select_cloud_protocol_azure').html('Microsoft Azure');
			$('#select_cloud_protocol_modbustcp_s').html('ModbusTCP Server');
		}
		this.mqtt.setLanguage(lang);
		this.azure.setLanguage(lang);
		this.modbustcp_s.setLanguage(lang);

	}

	refresh(){
		this.getConfig();
	}

	/**
	 * Extern called function which prevent to edit fields after logout.
	 */
	disableSet(){
		$('#btn_cloud_select').attr('disabled', true);
		$('#select_cloud_protocol').attr('disabled', true);

		this.mqtt.disableSet();
		this.azure.disableSet();
		this.modbustcp_s.disableSet();
	}

	/**
	 * Extern called function which allows to edit fields after login.
	 */
	enableSet(){
		$('#btn_cloud_select').attr('disabled', false);
		$('#select_cloud_protocol').attr('disabled', false);

		this.mqtt.enableSet();
		this.azure.enableSet();
		this.modbustcp_s.enableSet();
	}

	/**
	 * Reading all parameters form input and save these to the chip.
	 */
	saveConfig() {

		var params = {};
		params["select"] = $('#select_cloud_protocol').val();
		params["auth"] = this.auth.getCredentials();

		var that = this;
		this.jsonRPC.call('cloud.setConfig', params).then(function (result) {
			if(result["state"] == 0) {
				$('#modal_success_text_en').html("Your changes have been saved!");
				$('#modal_success_text_de').html("Die Einstellungen wurden gespeichert!");
				$('#modal_success').modal('show');
				setTimeout(function(){
					$('#modal_success').modal('hide');
				}, 1000);
			}else{
				$('#modal_error_text_en').html("Your changes have not been saved!");
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
			that.refresh();
		}, 1000);
	}

	/**
	 * reading all configuration parameters from the chip in to the input fields.
	 */
	getConfig() {
		var that= this;
		this.jsonRPC.call('cloud.getConfig').then(function (result) {

			$('#select_cloud_protocol').val(result["select"]);
			that.select = result["select"];

			//display only the selected
			switch(result["select"]){
				case "none":
					that.mqtt.setActive(false);
					that.azure.setActive(false);
					that.modbustcp_s.setActive(false);
					break;
				case "mqtt":
					that.mqtt.setActive(true);
					that.azure.setActive(false);
					that.modbustcp_s.setActive(false);
					that.mqtt.getConfig(result);
					break;
				case "azure":
					that.mqtt.setActive(false);
					that.azure.setActive(true);
					that.modbustcp_s.setActive(false);
					that.azure.getConfig(result);
					break;
				case "modbustcp_s":
					that.mqtt.setActive(false);
					that.azure.setActive(false);
					that.modbustcp_s.setActive(true);
					that.modbustcp_s.getConfig(result);
					break;
			}
		});
	}

	getStatus() {
		var that= this;

		if(this.select != "none"){
			this.jsonRPC.call('cloud.getConfig').then(function (result) {

				switch(that.select){
					case "mqtt":
						that.mqtt.getStatus(result);
						break;
					case "azure":
						that.azure.getStatus(result);
						break;
					case "modbustcp_s":
						that.modbustcp_s.getStatus(result);
						break;
				}
			});
		}

		//Wait 5 seconds before checking state
		var that = this;
		setTimeout(function(){
			that.getStatus();
		}, 5000);
	}
}