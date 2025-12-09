class Mqtt{
	constructor(jsonRPC, auth, cloud, lang){

		this.jsonRPC = jsonRPC;
		this.auth = auth;
		this.cloud = cloud;
		this.lang = lang;
	}

	run() {
		var that = this;

		$('#spacer_mqtt').load('html/mqtt.html #content', function(){
			that.setLanguage(that.lang);
			that.disableSSL();
			that.hideConfigTopic();

			//callback for button click
			$('#btn_mqtt_save').click(function(){
				that.saveConfig();
			});

			//enable/disable inputs if mqtt was activated/deactivated
			$('#input_mqtt_enable').change(function(){
				if ($('#input_mqtt_enable').is(':checked') == true){
					that.enableMqttConfig();
				}else{
					that.disableMqttConfig();
				}
			});

			//enable configurations for secured connection, if this is selected
			// check if a valid protocoll is selected
			$('#input_mqtt_protocol').change(function(){
				that.validMqttProtocol();
				if(that.connectionSecured()) that.enableSSL();
				else that.disableSSL();
			});

			// If "Enable JSON-RPC over MQTT" is checked -> show according input
			$('#input_mqtt_topic_rpc').change(function(){
				if($('#input_mqtt_topic_rpc').prop('checked')){
					if(that.connectionSecured()) that.showConfigTopic();
					else that.hideConfigTopic();
				}else that.hideConfigTopic();
			});

			//check if the given hostname is valid
			$('#input_mqtt_host').change(function(){
				that.validMqttHost();
			});

			//check if the given port is valid
			$('#input_mqtt_port').change(function(){
				that.validMqttPort();
			});

			$('#input_mqtt_crt').change(function(){
				that.uploadCert("mqtt_crt", "mqtt_ca.crt");
			});

			$('#input_mqtt_client_crt').change(function(){
				that.uploadCert("mqtt_client_crt", "mqtt.crt");
			});

			$('#input_mqtt_client_key').change(function(){
				that.uploadCert("mqtt_client_key", "mqtt.key")
			});
		});
	}

	setLanguage(lang){
		this.lang=lang;
		if(lang == "de"){
			$('.lang-de').show();
			$('.lang-en').hide();
			$('#input_mqtt_host').prop('placeholder','Hostname oder IP');
			$('#input_mqtt_protocol_none').html('auswählen ...');
		}else{
			$('.lang-de').hide();
			$('.lang-en').show();
			$('#input_mqtt_host').prop('placeholder','Hostname or IP');
			$('#input_mqtt_protocol_none').html('Select ...');
		}
	}

	uploadCert(dom, name){

		//FileList Objekt aus dem Input Element
		$('#input_'+dom).removeClass('is-invalid');
		$('#input_'+dom).removeClass('is-valid');
		var file = document.getElementById('input_'+dom).files[0];

		$('#label_'+dom).text(file.name);
		if(file.size > 16*1024){
			$('#input_'+dom).addClass('is-invalid');
			return;
		}

		Object.defineProperty(file, 'name', {
			writable: true,
			value: name
		});

		//init
		var formData = new FormData();
		this.client = new XMLHttpRequest();
		// we need a high timeout value -> init ota is very time intensive!
		this.client.timeout = 10*60*1000; // time in milliseconds

		//create an file headder
		formData.append(name, file);

		//define events
		//this.client.upload.onprogress = event => this.uploadProgress(event);
		this.client.onloadend = event => this.uploadDoneEvent(event, this.client, dom);
		this.client.onabort =  event => this.uploadAbort(event, dom);
		this.client.onerror = event => this.uploadAbort(event, dom);
		this.client.ontimeout = event => this.uploadAbort(event, dom);

		//open socket
		this.client.open("POST", "upload/"+file.name+"?username="+this.auth.getUsername()+"&token="+this.auth.getToken());
		this.client.send(formData);
	}

	uploadDoneEvent(event, client, name){
		if(client.status != 200) {
			$('#input_'+name).addClass('is-invalid');
		} else {
			$('#input_'+name).addClass('is-valid');
		}
		//this.saveConfig();
	}

	uploadAbort(event, name) {
		console.log(event);
		$('#input_'+name).addClass('is-invalid');
	}

	setActive(active){
		if(active == true) {
			$('#spacer_mqtt').show();
		}else{
			$('#spacer_mqtt').hide();
		}
	}

	/**
	 * Extern called function which prevent to edit fields after logout.
	 */
	disableSet(){
		//MQTT
		$('#input_mqtt_enable').attr('disabled', true);
		$('#btn_mqtt_save').attr('disabled', true);
		this.disableMqttConfig();
	}

	/**
	 * Extern called function which allows to edit fields after login.
	 */
	enableSet(){
		//MQTT
		$('#input_mqtt_enable').attr('disabled', false);
		$('#btn_mqtt_save').attr('disabled', false);
		var that = this;
		if ($('#input_mqtt_enable').is(':checked') == true){
			that.enableMqttConfig();
		}
	}

	/**
	 * Set All Inputs to read only and restore default configuration
	 */
	disableMqttConfig(){
		$('.form_mqtt_disable').attr('disabled', true);
		$('.form_mqtt_readonly').attr('readonly', true);
	}

	/**
	 * Remove all readonly tags from inputs.
	 */
	enableMqttConfig(){
		$('.form_mqtt_disable').attr('disabled', false);
		$('.form_mqtt_readonly').attr('readonly', false);
	}

	/**
	 *
	 */
	showCerts(){
		$('#input_mqtt_secured_div').slideDown('slow');
	}

	/**
	 * Show additional input fields, needed only for secured connection.
	 */
	enableSSL() {
		$('#input_mqtt_topic_rpc').attr('disabled', false);
		$('#input_mqtt_topic_rpc').prop('checked', false);

		this.showCerts();
	}

	/**
	 * Disable all inputs for secured connection
	 */
	disableSSL() {
		$('#input_mqtt_topic_rpc').prop('checked', false);
		$('#input_mqtt_topic_rpc').attr('disabled', true);
		this.hideConfigTopic();

		$('#input_mqtt_secured_div').slideUp('slow');
	}

	/**
	 * Show config topic.
	 */
	showConfigTopic(){
		$('#input_mqtt_topic_config_div').slideDown("slow");
	}

	/**
	 * Hide config topic.
	 */
	hideConfigTopic(){
		$('#input_mqtt_topic_config_div').slideUp("slow");
	}

	/**
	 * Check if the given MQTT address is for an secured connection.
	 */
	connectionSecured(){
		var protocol = $('#input_mqtt_protocol').val();
		if(protocol == "mqtts://" || protocol == "wss://") return true;
		return false;
	}

	/**
	 * Check if the given hostname is valid
	 */
	validMqttHost(){
		var ValidIpAddressRegex = "^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$";

		var ValidHostnameRegex = "^(([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\-]*[a-zA-Z0-9])\.)*([A-Za-z0-9]|[A-Za-z0-9][A-Za-z0-9\-]*[A-Za-z0-9])$";

		var host = $('#input_mqtt_host').val();

		if(!host.match(ValidIpAddressRegex) && !host.match(ValidHostnameRegex)){
			$('#input_mqtt_host').addClass('is-invalid');
			return false;
		}
		$('#input_mqtt_host').removeClass('is-invalid');

		return true;
	}

	/**
	 * Check invalid protocol
	 */
	validMqttProtocol(){
		if($('#input_mqtt_protocol').val() == ""){
			$('#input_mqtt_protocol').addClass('is-invalid');
			return false;
		}
		$('#input_mqtt_protocol').removeClass('is-invalid');
		return true;
	}

	/**
	 * Check if the given port is valid
	 */
	validMqttPort(){
		var ValidPortRegex = "^()([1-9]|[1-5]?[0-9]{2,4}|6[1-4][0-9]{3}|65[1-4][0-9]{2}|655[1-2][0-9]|6553[1-5])$";
    	if(!$('#input_mqtt_port').val().match(ValidPortRegex)){
			$('#input_mqtt_port').addClass('is-invalid');
			return false;
		}
		$('#input_mqtt_port').removeClass('is-invalid');

		return true;
	}

	/**
	 * Check if the file inputs has an is-invalid class
	 */
	validMqttCert(){
		//check upload success
		if($('#cb_mqtt_ca_crt').is(':checked') && $('#input_mqtt_ca_crt').hasClass("is-invalid")) return false;
		if($('#cb_mqtt_client_crt').is(':checked') && $('#input_mqtt_client_crt').hasClass("is-invalid")) return false;
		if($('#cb_mqtt_client_crt').is(':checked') && $('#input_mqtt_client_key').hasClass("is-invalid")) return false;
		return true;
	}

	/**
	 * Check if all necessay inputs are valid
	 */
	inputsValid(){
		//run each before we test, so that the browser can't optimize one out!
		this.validMqttProtocol();
		this.validMqttHost();
		this.validMqttPort();
		this.validMqttCert();
		if(this.validMqttProtocol() && this.validMqttHost() && this.validMqttPort() && this.validMqttCert()) return true;
		return false;
	}

	/**
	 * Reading all parameters form input and save these to the chip.
	 */
	saveConfig() {

		if(!this.inputsValid()){
			$('#modal_error_text_en').html("Your inputs are invalid!");
			$('#modal_error_text_de').html("Die Eingaben sind fehlerhaft!");
			$('#modal_error').modal('show');
			setTimeout(function(){
				$('#modal_error').modal('hide');
			}, 1000);
			return;
		}

		var params = {};
		params["active"] = $('#input_mqtt_enable').is(':checked');
		params["protocol"] = $('#input_mqtt_protocol').val();
		params["host"] = $('#input_mqtt_host').val();
		params["port"] = parseInt($('#input_mqtt_port').val());
		params["qos"] = $('#input_mqtt_qos').val();
		params["username"] = $('#input_mqtt_username').val();
		if($('#input_mqtt_password').val() != "PlaceholdVal")
			params["password"] = $('#input_mqtt_password').val();

		if($('#cb_mqtt_ca_crt').is(':checked')) params["ca_crt"] = true;
		if($('#cb_mqtt_client_crt').is(':checked')) params["client_crt"] = true;

		params["enable_config"] = $('#input_mqtt_topic_rpc').is(':checked');
		params["stateTopic"] = $('#input_mqtt_topic_state').val();
		params["commandTopic"] = $('#input_mqtt_topic_command').val();

		if($('#input_mqtt_topic_rpc').is(':checked')){
			params["configTopic"] = $('#input_mqtt_topic_config').val();
		}
		params["auth"] = this.auth.getCredentials();

		var that = this;
		this.jsonRPC.call('cloud.setConfig', params).then(function (result) {
			if(result["state"] == 0) {
				$('#modal_success_text_en').html("Your changes have been saved!");
				$('#modal_success_text_de').html("Die Einstellungen wurden gespeichert!");
				$('#modal_success').modal('show');
				setTimeout(function(){
					//hide success after 1 second
					$('#modal_success').modal('hide');
					if(params["protocol"] == "mqtts://" || params["protocol"] == "wss://"){
						setTimeout(function(){
							//reboot after 4 seonds
							that.reboot();
						}, 400);
					}
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
			that.cloud.getConfig();
		}, 1000);
	}

	/**
	 * reading all configuration parameters from the chip in to the input fields.
	 */
	getConfig(result) {
		//console.log("MQTT getConfig");

		//broker tab
		$('#input_mqtt_enable').prop( "checked", result["active"] );
		$('#input_mqtt_status').val(this.getStatusText(result));
		$('#input_mqtt_protocol').val(result["protocol"]);
		$('#input_mqtt_host').val(result["host"]);
		if(result["port"] == 0) $('#input_mqtt_port').val("");
		else $('#input_mqtt_port').val(result["port"].toString());
		$('#input_mqtt_qos').val(result["qos"]);
		$('#input_mqtt_username').val(result["username"]);
		if(result["username"] != "")
			$('#input_mqtt_password').val("PlaceholdVal"); //placeholder
		else
			$('#input_mqtt_password').val("");

		if(this.connectionSecured()){
			$('#cb_mqtt_ca_crt').prop( "checked", result["ca_crt"]);
			$('#cb_mqtt_client_crt').prop( "checked", result["client_crt"]);
			$('#input_mqtt_topic_rpc').prop( "checked", result["enable_config"]);
			this.showCerts();
		}else{
			$('#cb_mqtt_ca_crt').prop( "checked", false);
			$('#cb_mqtt_client_crt').prop( "checked", false);
			$('#input_mqtt_topic_rpc').prop( "checked", false);
		}

		//set defaults for cert input
		$('#label_mqtt_ca_crt').text($('#label_mqtt_ca_crt').attr('data-defaultText'));
		$('#label_mqtt_client_crt').text($('#label_mqtt_client_crt').attr('data-defaultText'));
		$('#label_mqtt_client_key').text($('#label_mqtt_client_key').attr('data-defaultText'));
		$('.form_mqtt_crt').removeClass('is-invalid');
		$('.form_mqtt_crt').removeClass('is-valid');

		//Topic Tab
		$('#input_mqtt_deviceID').val(result["deviceID"]);
		$('#input_mqtt_topic_state').val(result["stateTopic"]);
		$('#input_mqtt_topic_command').val(result["commandTopic"]);
		$('#input_mqtt_topic_config').val(result["configTopic"]);
	}

	getStatus(result) {
		$('#input_mqtt_status').val(this.getStatusText(result));
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

	reboot(){
		var params = {}
		params["auth"] = this.auth.getCredentials();
		this.jsonRPC.call('system.reboot', params).then(function (result) {});
		if(this.lang == "de"){
			alert("Bitte die Seite neu laden, nachdem das Gerät neu gestartet ist!\n");
		}else{
			alert("Please reload the page, after the device has been rebooted!\n");
		}
	}

}