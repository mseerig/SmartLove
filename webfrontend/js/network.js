class Network{
	constructor(jsonRPC, auth, lang, cloud){

		this.lang =lang;
		this.jsonRPC = jsonRPC;
		this.auth = auth;
		this.octet = '(?:25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9][0-9]|[0-9])';
		this.ip = '(?:' + this.octet + '\\.){3}' + this.octet;
		this.ipRE = new RegExp('^' + this.ip + '$');
		this.cloud = cloud;

	}
	run() {
		var that = this;
		$('#tab_network').load('html/network.html #content', function(){
			that.setLanguage(that.lang);
			that.refreshEthernetStatus();
			that.refreshWifiStatus();

			$('#btn_eth_save').click(function(){that.saveEthernetConfig();});
			$('#btn_wifi_save').click(function(){that.saveWifiConfig();});
			$('#cb_eth_enbable').change(function(){that.updateEthEnableCb();});
			$('#radio_eth_dhcp').change(function(){that.updateEthEnableCb();});
			$('#radio_eth_staticIP').change(function(){that.updateEthEnableCb();});
			$('#radio_eth_dns_auto').change(function(){that.updateEthEnableCb();});
			$('#radio_eth_dns_manual').change(function(){that.updateEthEnableCb();});
			$('#cb_wifi_enable').change(function(){that.updateWifiEnableCb();});

			that.updateEthEnableCb();
			that.updateWifiEnableCb();

			that.refresh();

			$('#input_eth_staticIP').change(function(){
				that.validateStaticIP();
			});
			$('#input_eth_staticGateway').change(function(){
				that.validateStaticGateway();
			});
			$('#input_eth_staticNetmask').change(function(){
				that.validateStaticNetmask();
			});
			that.cloud.run();
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

	validateIP(object){
		if( !this.ipRE.test( $(object).val() ) ){
			$(object).addClass('is-invalid');
			return false;
		}
		$(object).removeClass('is-invalid');

		return true;
	}

	inputsValid(){
		//prevent browser optimization
		var valid = true;
		if($('#radio_eth_staticIP').is(':checked') == true){
			valid &= this.validateIP('#input_eth_staticIP');
			valid &= this.validateIP('#input_eth_staticGateway');
			valid &= this.validateIP('#input_eth_staticNetmask');
		}
		if($('#radio_eth_dns_manual').is(':checked') == true){
			valid &= this.validateIP('#input_eth_dns_main');
			valid &= this.validateIP('#input_eth_dns_fallback');
		}
		return valid;
	}

	disableSet(){
		$('#cb_eth_enbable').attr('disabled', true);
		$('#cb_wifi_enable').attr('disabled', true);
		$('#input_eth_staticIP').prop('disabled', true);
		$('#input_eth_staticNetmask').prop('disabled', true);
		$('#input_eth_staticGateway').prop('disabled', true);
		$('#radio_eth_dhcp').prop('disabled', true);
		$('#radio_eth_staticIP').prop('disabled', true);
		$('#input_wifi_ssid').prop('disabled', true);
		$('#input_wifi_password').prop('disabled', true);
		$('#btn_eth_save').attr('disabled', true);
		$('#btn_wifi_save').attr('disabled', true);
		$('#btn_wifi_save').attr('disabled', true);
		$('#radio_eth_dns_auto').attr('disabled', true);
		$('#radio_eth_dns_manual').attr('disabled', true);
		$('#input_eth_dns_main').attr('disabled', true);
		$('#input_eth_dns_fallback').attr('disabled', true);
	}

	enableSet(){
		$('#cb_eth_enbable').attr('disabled', false);
		$('#cb_wifi_enable').attr('disabled', false);
		$('#btn_eth_save').attr('disabled', false);
		$('#btn_wifi_save').attr('disabled', false);
		this.updateEthEnableCb();
		this.updateWifiEnableCb();
	}

	refresh(){
		this.getEthernetConfig();
		this.getWifiConfig();
	}

	refreshEthernetStatus() {
		this.jsonRPC.call('ethernet.getStatus').then(function (result) {

			$("#input_eth_status").val(that.getStatusText(result));
			$("#input_eth_ip").val(result.ip);
			$("#input_eth_gateway").val(result.gateway);
		});

		var that = this;
		setTimeout(function(){
			that.refreshEthernetStatus();
		}, 5000);
	}

	refreshWifiStatus() {
		this.jsonRPC.call('wifi.getStatus').then(function (result) {

			$("#input_wifi_status").val(that.getStatusText(result));
			$("#input_wifi_ip").val(result.ip);
			$("#input_wifi_gateway").val(result.gateway);
		});

		var that = this;
		setTimeout(function(){
			that.refreshWifiStatus();
		}, 5000);
	}

	getEthernetConfig() {
		var that = this;
		this.jsonRPC.call('ethernet.getConfig').then(function(result){

			$('#cb_eth_enbable').prop( "checked", result["active"] );

			$('#radio_eth_dhcp').prop( "checked", result["useDHCP"] );
			$('#radio_eth_staticIP').prop( "checked", !result["useDHCP"] );

			if (!result["useDHCP"]) {
				$("#input_eth_staticIP").val(result["ip"]);
				$("#input_eth_staticGateway").val(result["gateway"]);
				$("#input_eth_staticNetmask").val(result["netmask"]);
			}

			$('#radio_eth_dns_auto').prop( "checked", !result["user_dns"] );
			$('#radio_eth_dns_manual').prop( "checked", result["user_dns"] );
			$("#input_eth_dns_main").val(result.dns_main);
			$("#input_eth_dns_fallback").val(result.dns_fallback);

			that.updateEthEnableCb();
		});
	}

	getWifiConfig() {
		var that = this;
		this.jsonRPC.call('wifi.getConfig').then(function(result){

			$('#cb_wifi_enable').prop( "checked", result["active"] );
			$("#input_wifi_ssid").val(result["ssid"]);
			$("#input_wifi_password").val("");

			that.updateWifiEnableCb();
		});
	}

	saveEthernetConfig() {

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

		params["active"] = $('#cb_eth_enbable').is(':checked');
		params["useDHCP"] = $('#radio_eth_dhcp').is(':checked');

		if(!params["useDHCP"]){
			params["ip"] = $("#input_eth_staticIP").val();
			params["gateway"] = $("#input_eth_staticGateway").val();
			params["netmask"] = $("#input_eth_staticNetmask").val();
		}

		params["user_dns"] = $('#radio_eth_dns_manual').is(':checked');
		if(params["user_dns"]){
			params["dns_main"] = $("#input_eth_dns_main").val();
			params["dns_fallback"] = $("#input_eth_dns_fallback").val();
		}

		params["auth"] = this.auth.getCredentials();

		var that = this;
		this.jsonRPC.call('ethernet.setConfig', params).then(function (result) {

			if(result['state'] == 0) {
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
		setTimeout(function(){
			that.getEthernetConfig();
		}, 1000);
	}

	saveWifiConfig() {
		var params = {};

		params["active"] = $('#cb_wifi_enable').is(':checked');
		params["ssid"] = $("#input_wifi_ssid").val();
		params["password"] =  $("#input_wifi_password").val();
		params["auth"] = this.auth.getCredentials();

		var that = this;
		this.jsonRPC.call('wifi.setConfig', params).then(function (result) {

			if(result['state'] == 0) {
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
		setTimeout(function(){
			that.getWifiConfig();
		}, 1000);
	}

	updateEthEnableCb(){
		if ($('#cb_eth_enbable').is(':checked') == true) {
			$('#cb_wifi_enable').prop('checked', false );
			$('#radio_eth_dhcp').prop('disabled', false);
			$('#radio_eth_staticIP').prop('disabled', false);
			$('#radio_eth_dns_auto').attr('disabled', false);
			$('#radio_eth_dns_manual').attr('disabled', false);
			if ($('#radio_eth_dhcp').is(':checked') == true) {
				$('#input_eth_staticIP').prop('disabled', true);
				$('#input_eth_staticNetmask').prop('disabled', true);
				$('#input_eth_staticGateway').prop('disabled', true);
				$('#input_eth_staticIP').val("");
				$('#input_eth_staticNetmask').val("");
				$('#input_eth_staticGateway').val("");
			} else {
				$('#input_eth_staticIP').prop('disabled', false);
				$('#input_eth_staticNetmask').prop('disabled', false);
				$('#input_eth_staticGateway').prop('disabled', false);
			}
			if($('#radio_eth_dns_manual').is(':checked')){
				$('#input_eth_dns_main').attr('disabled', false);
				$('#input_eth_dns_fallback').attr('disabled', false);
			}else{
				$('#input_eth_dns_main').attr('disabled', true);
				$('#input_eth_dns_fallback').attr('disabled', true);
				$('#input_eth_dns_main').val("");
				$('#input_eth_dns_fallback').val("");
			}
		} else {
			$('#input_eth_staticIP').prop('disabled', true);
			$('#input_eth_staticNetmask').prop('disabled', true);
			$('#input_eth_staticGateway').prop('disabled', true);
			$('#radio_eth_dhcp').prop('disabled', true);
			$('#radio_eth_staticIP').prop('disabled', true);
			$('#radio_eth_dns_auto').attr('disabled', true);
			$('#radio_eth_dns_manual').attr('disabled', true);
			$('#input_eth_dns_main').attr('disabled', true);
			$('#input_eth_dns_fallback').attr('disabled', true);
		}
	}

	updateWifiEnableCb(){
		if ($('#cb_wifi_enable').is(':checked') == true) {
			$('#cb_eth_enbable').prop('checked', false);
			$('#input_wifi_ssid').prop('disabled', false);
			$('#input_wifi_password').prop('disabled', false);
		} else {
			$('#input_wifi_ssid').prop('disabled', true);
			$('#input_wifi_password').prop('disabled', true);
		}
	}

	/**
	 * Reading connection state from chip an show these in an readonly input.
	 */
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