
var timeoutStarted = false;
class System{
	constructor(jsonRPC, auth, lang, settings){
		
		this.jsonRPC = jsonRPC;
		this.auth = auth;
		this.client = null;
		this.file = null;
		this.progress = 0;
		this.loadComplete = false;
		this.setDisable = false;
		this.lang = lang;
		this.uptimeConterRunning = false;
		this.settings = settings;
	}
	
	run(){
		//load html content
		var that = this;
		$('#tab_system').load('html/system.html #content',function(){
			that.setLanguage(that.lang);
			//loadTimezones
			$.getJSON("assets/timezones.json", function(json) {
				var x;
				for (x in json) {
					$("#input_system_timezone").append(new Option(x, json[x]));
				}
				that.refresh();
			});

			//callback for button click
			$('#btn_system_time').click(function(){
				that.setTimeConfig();
			});

			//callback for button click
			$('#btn_system_ap').click(function(){
				that.setAPConfig();
			});

			//callback for button click
			$('#btn_system_info').click(function(){
				that.setDeviceInfo();
			});

			// check if a valid ssid is given
			$('#input_system_ap_ssid').change(function(){
				that.validSSID();
			});

			// check if a valid pass is given
			$('#input_system_ap_pass').change(function(){
				that.validPassword();
			});

			//callback for button click
			$('#btn_system_reboot').click(function(){
				that.reboot();
			});

			//callback for button click
			$('#btn_system_factoryReset').click(function(){
				that.factoryReset();
			});

			$('#input_system_sntp_enable').change(function(){
				if($('#input_system_sntp_enable').is(':checked')){
					$('#input_system_manual_time').prop( "checked", false);
				}
				that.handleSNTPenable();
			});

			$('#input_system_manual_time').change(function(){
				if($('#input_system_manual_time').is(':checked')){
					$('#input_system_sntp_enable').prop( "checked", false);
				}
				that.handleSNTPenable();
			});

			$('#input_system_sntp_enable').change(function(){
				if($('#input_system_sntp_enable').is(':checked')){
					$('#input_system_sntpServer').attr('readonly', false);
				}else{
					$('#input_system_sntpServer').attr('readonly', true);
				}
			});

			// check if a valid pass is given
			$('#input_system_time').change(function(){
				that.validTimestamp();
			});

			$('#selcect_ota_method').change(function(){that.selcetMethod();});
			$('#btn_ota_submitBtn').click(function(){that.submitBtnClick();});
			$('#input_ota_file').change(function(){that.fileChange();});

			$('#input_ota_url').change(function(){
				that.validUri();
			});
			that.getDeviceInfo();

			that.settings.run();
		});
	}

	refresh(){
		this.getTimeConfig();
		this.getAPConfig();
		this.getDeviceInfo();
	}

	setLanguage(lang){
		this.lang=lang;
		if(lang == "de"){
			$('.lang-de').show();
			$('.lang-en').hide();
			$('#input_system_sntpServer').prop('placeholder','SNTP Server');
			$('#selcect_ota_method_none').html('auswählen ...');
			$('#selcect_ota_method_file').html('Update-Datei hochladen');
			$('#selcect_ota_method_link').html('Update-Link verwenden');
		}else{
			$('.lang-de').hide();
			$('.lang-en').show();
			$('#input_system_sntpServer').prop('placeholder','SNTP server');
			$('#selcect_ota_method_none').html('Select ...');
			$('#selcect_ota_method_file').html('File upload');
			$('#selcect_ota_method_link').html('Use update URL');
		}
	}

	getDeviceInfo(){
		//run update if load done
		var that = this;
		this.jsonRPC.call('system.getInfo').then(function (result) {
			$('#system_moduleType').html(result["moduleType"]);
			$('#system_device_name').val(result["name"]);
			$('#system_device_location').val(result["location"]);
			$('#system_deviceID').html(result["deviceID"]);
			//$('#system_productionDate').html(result["productionDate"]);
			$('#system_productVersion').html(result["productVersion"]);
			$('#system_hardwareVersion').html(result["hardwareVersion"]);
			$('#system_firmwareVersion').html(result["firmwareVersion"]);
			$('#system_coreVersion').html(result["coreVersion"]);

			$('#home_device_title').html(result["moduleType"]);
			if(result["location"] && result["name"]){
				$('#home_device_subtitle').text(result["name"] + " - " + result["location"]);
			}
			if(result["location"] && !result["name"]){
				$('#home_device_subtitle').text(result["location"]);
			}
			if(!result["location"] && result["name"]){
				$('#home_device_subtitle').text(result["name"]);
			}
			if(!result["location"] && !result["name"]){
				$('#home_device_subtitle').text("");
			}
			
			that.uptime = result["upTime"];

			if(!that.uptimeConterRunning){
				that.uptimeConterRunning = true;
				that.refreshUptime();
			}
		});
	}

	setDeviceInfo(){
		var params = {};
		params["name"] = $('#system_device_name').val();
		params["location"] = $('#system_device_location').val();
		params["auth"] = this.auth.getCredentials();

		var that = this;
		this.jsonRPC.call('system.setInfo', params).then(function (result) {
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
			that.getDeviceInfo();
		}, 1000);
	}

	refreshUptime(){
		var sec_num = parseInt(this.uptime, 10); // don't forget the second param
		var sec_decrement = sec_num;

		var days    = Math.floor(sec_decrement / (3600 * 24));
		sec_decrement = sec_decrement - days*24*3600;

		var hours   = Math.floor(sec_decrement  / 3600);
		sec_decrement = sec_decrement - hours*3600;

		var minutes = Math.floor(sec_decrement / 60);
		sec_decrement = sec_decrement - minutes*60;
		
		var seconds = sec_decrement;

		var str = "";
		if(this.lang == "en"){
			if (sec_num >= 3600*24) {str += days + " days ";}
			if (sec_num >= 3600)    {str += hours + " hours ";}
			if (sec_num >= 60)      {str += minutes + " minutes ";}
			str += seconds + " seconds";
		}else{
			if (sec_num >= 3600*24) {str += days + " Tage ";}
			if (sec_num >= 3600)    {str += hours + " Stunden ";}
			if (sec_num >= 60)      {str += minutes + " Minuten ";}
			str += seconds + " Sekunden";
		}

		$('#system_upTime').html(str);

		var that = this;
		setTimeout(function(){
			that.uptime += 1;
			that.refreshUptime();
		}, 1000);
	}

	disableSet(){
		$('.form_system').attr('readonly', true);
		$('.form_system_ap').attr('readonly', true);
		$('.form_system_disable').attr('disabled', true);
		$('#input_system_time').attr('readonly', true);
		$('#input_system_sntpServer').attr('readonly', true);
		this.setDisable = true;
	}

	enableSet(){
		$('.form_system').attr('readonly', false);
		$('.form_system_ap').attr('readonly', false);
		$('.form_system_disable').attr('disabled', false);
		this.handleSNTPenable();
		this.setDisable = false;
	}

	handleSNTPenable(){
		if(!$('#input_system_manual_time').is(':checked')){
			$('#input_system_time').attr('readonly', true);
		}else{
			$('#input_system_time').attr('readonly', false);
		}

		if($('#input_system_sntp_enable').is(':checked')){
			$('#input_system_sntpServer').attr('readonly', false);
		}else{
			$('#input_system_sntpServer').attr('readonly', true);
		}
	}

	validSSID(){
		if($('#input_system_ap_ssid').val() == ""){
			$('#input_system_ap_ssid').addClass('is-invalid');
			return false;
		}
		$('#input_system_ap_ssid').removeClass('is-invalid');
		return true;
	}

	validPassword(){
		if($('#input_system_ap_pass').val() == "" || $('#input_system_ap_pass').val().length <8){
			$('#input_system_ap_pass').addClass('is-invalid');
			return false;
		}
		$('#input_system_ap_pass').removeClass('is-invalid');
		return true;
	}

	validTimestamp(){
		var ValidTimestampRegex = "^[0-9]{4}-(0[1-9]|1[0-2])-(0[1-9]|[1-2][0-9]|3[0-1]) (2[0-3]|[01][0-9]):[0-5][0-9]:[0-5][0-9]$";
    	if(!$('#input_system_time').val().match(ValidTimestampRegex)){
			$('#input_system_time').addClass('is-invalid');
			return false;
		}
		$('#input_system_time').removeClass('is-invalid');

		return true;
	}

	timestampToUnix(timestamp){
		//var timestamp = "2020-03-23 22:40:40";
		timestamp = timestamp.replace(".", "-");
		timestamp = timestamp.replace(" ", "T");
		var date = new Date(timestamp);
		var time = date/1000;
		time-= date.getTimezoneOffset()*60; //hier muss man die localtime vom browser wieder wegnehmen
		console.log(time);
		return time;
	}

	getTimeConfig(){
		var that = this;
		this.jsonRPC.call('system.getTime').then(function (result) {

			$('#input_system_sntp_enable').prop( "checked", result["enableSNTP"] );

			$('#input_system_sntpServer').val(result["sntpServer"]);
			$('#input_system_enableTimeOutput').prop( "checked", result["enableTimeOutput"] );

			//if location in list
			if ($('#input_system_timezone').has('option:contains('+result["location"]+')').length){
				//select that location
				$("#input_system_timezone option:contains("+result["location"]+")").prop("selected", true);
			}else{
				//show the user-defined
				$("#input_system_timezone").append(new Option(result["location"], result["timezone"]));
				$("#input_system_timezone option:contains("+result["location"]+")").prop("selected", true);
			}

			$('#input_system_manual_time').prop( "checked", false );
			that.handleSNTPenable();

			var formattedTimestamp = result["timestamp"].substring(0, 19).replace("T"," ");

			$('#input_system_time').val(formattedTimestamp);

			if(result["rtcWorking"]){
				$('#warning_system_rtc').hide("slow");
			}else{
				$('#warning_system_rtc').show("slow");
			}
		});
	}

	getAPConfig(){
		this.jsonRPC.call('system.getApConfig').then(function (result) {
			$('#input_system_ap_alwaysActive').prop( "checked", result["alwaysActive"] );
			$('#input_system_ap_ssid').val(result["ssid"]);
		});
	}

	setTimeConfig(){
		var params = {};
		if($('#input_system_manual_time').is(':checked')){
			if(!this.validTimestamp()){
				$('#modal_error_text_en').html("Your changes have not been saved!");
				$('#modal_error_text_de').html("Die Einstellungen wurden nicht gespeichert!");
				$('#modal_error').modal('show');
				setTimeout(function(){
					$('#modal_error').modal('hide');
				}, 1000);
				return;
			}
			params["manualTime"] = this.timestampToUnix($('#input_system_time').val());
		}

		params["enableSNTP"] = $('#input_system_sntp_enable').is(':checked');
		if(params["enableSNTP"]){
			if(!this.validSntpServer()){
				$('#modal_error_text_en').html("Your changes have not been saved!");
				$('#modal_error_text_de').html("Die Einstellungen wurden nicht gespeichert!");
				$('#modal_error').modal('show');
				setTimeout(function(){
					$('#modal_error').modal('hide');
				}, 1000);
				return;
			}
			params["sntpServer"] = $("#input_system_sntpServer").val();
		}
		params["enableTimeOutput"] = $('#input_system_enableTimeOutput').is(':checked');
		params["timezone"] = $('#input_system_timezone').val();
		params["location"] = $('#input_system_timezone option:selected').text();
		params["auth"] = this.auth.getCredentials();

		var that = this;
		this.jsonRPC.call('system.setTime', params).then(function (result) {
			if(result["state"] == 0) {
				$('#modal_success_text_en').html("Your changes have been saved!");
				$('#modal_success_text_de').html("Die Einstellungen wurden gespeichert!");
				$('#modal_success').modal('show');
				setTimeout(function(){
					$('#modal_success').modal('hide');
				}, 1000);

				//reset manaul button
				$('#input_system_manual_time').prop( "checked", false );
				that.handleSNTPenable();
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
			that.getTimeConfig();
		}, 3000);
	}

	setAPConfig(){
		var params = {};

		if(!this.validSSID() || !this.validPassword()){
			$('#modal_error_text_en').html("Your changes have not been saved!");
			$('#modal_error_text_de').html("Die Einstellungen wurden nicht gespeichert!");
			$('#modal_error').modal('show');
			setTimeout(function(){
				$('#modal_error').modal('hide');
			}, 1000);
			return;
		}
		params["ssid"] = $('#input_system_ap_ssid').val();

		//only if changed
		if($('#input_system_ap_pass').val() != "PlaceholdVal")
			params["password"] = $('#input_system_ap_pass').val();

		params["alwaysActive"] = $('#input_system_ap_alwaysActive').is(':checked');
		params["auth"] = this.auth.getCredentials();

		var that = this;
		this.jsonRPC.call('system.setApConfig', params).then(function (result) {
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
			that.getAPConfig();
		}, 1000);
	}

	selcetMethod(){
		//set defaults for update button
		this.uploadDone = false;

		//make all invisible
		$('#div_ota_file').hide();
		$('#div_ota_fileProgress').hide();
		$('#div_ota_link').hide();
		this.setInfoBox("none");

		//display only the selected
		switch($('#selcect_ota_method').val()){
			case "file":
				$('#div_ota_file').show();
				$('#div_ota_fileProgress').show();
				break;
			case "link":
				$('#div_ota_link').show();
				break;
		}
	}

	submitBtnClick(){
		switch($('#selcect_ota_method').val()){
			case "file":
				this.uploadFile();
				break;
			case "link":
				this.updateWithLink();
				break;
		}
	}

	updateWithLink(){

		if(!this.validUri()){
			$('#modal_error_text_en').html("Your inputs are invalid!");
			$('#modal_error_text_de').html("Die Eingaben sind fehlerhaft!");
			$('#modal_error').modal('show');
			setTimeout(function(){
				$('#modal_error').modal('hide');
			}, 1000);
			return;
		}

		this.setInfoBox("init");
		var params = {}
		params["url"] =  $("#input_ota_url").val();
		params["auth"] = this.auth.getCredentials();

		var that = this;
		this.jsonRPC.call('system.firmwareUpdate', params).then(function(result){
			if(result['state'] == 0){
				that.uploadDone = true;
				that.setInfoBox("done");
			} else {
				$('#box_ota_error').html("<strong>Error:</strong> " + result['info']);
				that.setInfoBox("error");

				that.auth.handleError(result["state"]);
			}
		});
	}

	fileChange(){
		this.uploadDone = false;
		$('#btn_ota_submitBtn').attr('readonly', false);

		//FileList Objekt aus dem Input Element
		var fileList = document.getElementById("input_ota_file").files;
		this.file = fileList[0];

		//File Objekt nicht vorhanden = keine Datei ausgewählt oder vom Browser nicht unterstützt
		if(!this.file)
			return;

		//reset progress bar
		this.progress = 0;
		this.setProgress(this.progress);
	}

	uploadFile(){

		this.setInfoBox("init");

		//init
		var formData = new FormData();
		this.client = new XMLHttpRequest();
		// we need a high timeout value -> init ota is very time intensive!
		this.client.timeout = 10*60*1000; // time in milliseconds
		this.client.handleError

		//create an file headder
		formData.append("datei", this.file);

		//define events
		this.client.upload.onprogress = event => this.uploadProgress(event);
		this.client.onloadend = event => this.uploadDoneEvent(event, this.client);
		this.client.onabort =  event => this.uploadAbort(event);
		this.client.onerror = event => this.uploadAbort(event);
		this.client.ontimeout = event => this.uploadAbort(event);

		//open socket
		this.client.open("POST", "ota?username="+this.auth.getUsername()+"&token="+this.auth.getToken());
		this.client.send(formData);
	}

	uploadProgress(event){
		/* This progress bar start's not at 0 because of the "huge" capacity from the
		** socket_in_buffer of the hardware.
		** About 24% of the data will be instant transmitted to the  hardware, but will
		** not processed so faw. After the ota init process, the upload will proceed.
		** The chip is now reading the data form his buffer and will sent new ACK flags
		** back to the client... */
		var updateStartsActuallyAt = (530000 / this.file.size) *100; // 24% (see above)
		this.progress = Math.round(event.loaded / this.file.size * 100);
		if(this.progress > 100) this.progress = 100;
		this.setProgress();

		if(this.progress <= updateStartsActuallyAt)
			this.setInfoBox("init");
		else if(updateStartsActuallyAt<this.progress && this.progress<100)
			this.setInfoBox("flash");
		else if(this.progress == 100)
			this.setInfoBox("validate");
	}

	uploadDoneEvent(event, client){

		if(client.response != "OK") {
			this.setInfoBox("error");
			$('#box_ota_error').html("<strong>Error:</strong> "+client.response);
		} else {
			this.uploadDone = true;
			this.setInfoBox("done");
			$('#btn_ota_submitBtn').attr('readonly', true);
		}
	}

	uploadAbort(event) {
		console.log(event);
		this.setInfoBox("error");
		$('#box_ota_error').html("<strong>Error:</strong> "+client.response);
	}

	setProgress(){
		$('#bar_ota_progress').attr("style", "width: "+this.progress+"%;");
		$('#bar_ota_progress').html(this.progress+"%");
	}

	setInfoBox(type){
		//turn all off
		$('#box_ota_init').hide();
		$('#box_ota_flash').hide();
		$('#box_ota_validate').hide();
		$('#box_ota_success').hide();
		$('#box_ota_error').hide();

		//turn only the right on
		switch(type){
			case "init": 	 $('#box_ota_init').show(); break;
			case "flash":	 $('#box_ota_flash').show(); break;
			case "validate": $('#box_ota_validate').show(); break;
			case "done":     $('#box_ota_success').show(); break;
			case "error":    $('#box_ota_error').show(); break;
		}
	}

	reboot(){
		$('#modal_reboot').modal('hide');
		
		var params = {}
		params["auth"] = this.auth.getCredentials();
		this.jsonRPC.call('system.reboot', params).then(function () {});

		$('#modal_success_text_en').html("This site will reload after 10 seconds.");
		$('#modal_success_text_de').html("Die Seite wird in 10 Sekunden automatisch neu geladen.");
		$('#modal_success').modal('show');
		setTimeout(function(){
			$('#modal_success').modal('hide');
			location.reload();
		}, 10000);
	}

	factoryReset(){
		$('#modal_factoryReset').modal('hide');
		
		var params = {}
		params["auth"] = this.auth.getCredentials();
		this.jsonRPC.call('system.factoryReset', params).then(function () {});

		$('#modal_success_text_en').html("This site will reload after 10 seconds.");
		$('#modal_success_text_de').html("Die Seite wird in 10 Sekunden automatisch neu geladen.");
		$('#modal_success').modal('show');
		setTimeout(function(){
			$('#modal_success').modal('hide');
			location.reload();
		}, 10000);
	}

	/**
	 * Check if the given uri is valid
	 */
	validUri(){
		//var ValidUriRegex = /^(?:http(s)?:\/\/)[\w.-]+(?:\.[\w\.-]+)+[\w\-\._~:/?#[\]@!\$&'\(\)\*\+,;=.]+$/gm;
		var ValidUriRegex = /^(?:http?:\/\/)[\w.-]+(?:\.[\w\.-]+)+[\w\-\._~:/?#[\]@!\$&'\(\)\*\+,;=.]+$/gm;
    	if(!$('#input_ota_url').val().match(ValidUriRegex)){
			$('#input_ota_url').addClass('is-invalid');
			return false;
		}
		$('#input_ota_url').removeClass('is-invalid');

		return true;
	}

	validSntpServer(){
		var ValidIpAddressRegex = "^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$";

		var ValidHostnameRegex = "^(([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\-]*[a-zA-Z0-9])\.)*([A-Za-z0-9]|[A-Za-z0-9][A-Za-z0-9\-]*[A-Za-z0-9])$";

		var host = $('#input_system_sntpServer').val();

		if(!host.match(ValidIpAddressRegex) && !host.match(ValidHostnameRegex)){
			$('#input_system_sntpServer').addClass('is-invalid');
			return false;
		}
		$('#input_system_sntpServer').removeClass('is-invalid');

		return true;
	}

}