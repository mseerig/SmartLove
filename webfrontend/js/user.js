class User{
	constructor(jsonRPC, lang){
		this.jsonRPC = jsonRPC;

		this.username = "";
		this.token = "";
		this.lang = lang;

		this.authLevel = 0;

	}
	run() {
		var that = this;
		$('#user').load('html/user.html #content',function(){
			that.setLanguage(that.lang);
			//show login popup
			$('#div_user_main').hide();
			$('#box_user_error').hide();
			$('#btn_user_login').click(function(){that.login();});

			//load main
			//table
			$('#table_user_username').html(that.username);
			$('#table_user_apiToken').html(that.token);

			//logout
			$('#btn_user_logout').click(function(){that.logout();});

			//change password
			$('#box_user_newPassword_error').hide();
			$('#box_user_newPassword_success').hide();
			$('#btn_user_change_credentials').click(function(){that.changeCredentials();});

			//validation
			$('#input_user_newUsername').change(function(){
				if($(this).val() != "" && $(this).val().length >= 5){
					$(this).removeClass('is-invalid');
					$(this).addClass('is-valid');
				}else{
					$(this).removeClass('is-valid');
					$(this).addClass('is-invalid');
				}
			});
			//validation

			$('#input_user_newPassword').change(function(){
				var regularExpression = "^(.*[a-zA-Z])(.*[0-9])$";
				var re = new RegExp( regularExpression);

				if($(this).val() != "" && re.test($(this).val()) && $(this).val().length >= 5){
					$(this).removeClass('is-invalid');
					$(this).addClass('is-valid');
				}else{
					$(this).removeClass('is-valid');
					$(this).addClass('is-invalid');
				}
			});
			//validation
			$('#input_user_newPasswordRepeat').change(function(){
				if($(this).val()!= "" && $(this).val() == $('#input_user_newPassword').val()){
					$(this).removeClass('is-invalid');
					$(this).addClass('is-valid');
				}else{
					$(this).removeClass('is-valid');
					$(this).addClass('is-invalid');
				}
			});
		});
	}

	setLanguage(lang){
		this.lang=lang;
		if(lang == "de"){
			$('.lang-de').show();
			$('.lang-en').hide();
			$('#input_user_username').prop('placeholder','Benutzername');
			$('#input_user_password').prop('placeholder','Passwort');
		}else{
			$('.lang-de').hide();
			$('.lang-en').show();
			$('#input_user_username').prop('placeholder','Username');
			$('#input_user_password').prop('placeholder','Password');
		}
	}

	login() {
		var params = {};
		params["username"] = $('#input_user_username').val().replace(" ", "");
		params["password"] = $('#input_user_password').val().replace(" ", "");
		params["username"] = params["username"].replace('\t', '');
		params["password"] = params["password"].replace('\t', '');
		this.username = params["username"];

		var that = this;
		this.jsonRPC.call('authenticator.login', params).then(function (result) {
			if(result["state"] == 0) {
				that.token = result["apiToken"];
				that.authLevel = result["authLevel"];
				$('#div_user_login').hide();
				setTimeout(function(){
					$('#div_user_main').show();
				}, 1000);
				$('#btn_span_user').html("My Account");
				//change color of login button
				$('#user-tab').removeClass("login-false");
				$('#user-tab').addClass("login-true");
				$('#table_user_username').html(that.username);
				$('#table_user_apiToken').html(that.token);

				$('#home-tab').click(); //$('a[href=href="#home"]').click();
				$('#modal_error_text_en').html("You are logged in!");
				$('#modal_error_text_de').html("Sie sind angemeldet!");
				$('#modal_success').modal('show');
				setTimeout(function(){
					$('#modal_success').modal('hide');
				}, 1000);
			}else{
				$('#modal_error_text_en').html(result["info"]);
				$('#modal_error_text_de').html("Etwas ist schief gegangen..");
				$('#modal_error').modal('show');
				setTimeout(function(){
					$('#modal_error').modal('hide');
				}, 1000);
			}
		});
	}

	changeCredentials(){
		$('#input_user_newUsername').change();
		$('#input_user_newPassword').change();
		$('#input_user_newPasswordRepeat').change();

		if(    $('#input_user_newUsername').hasClass('is-invalid')
			|| $('#input_user_newPassword').hasClass('is-invalid')
			|| $('#input_user_newPasswordRepeat').hasClass('is-invalid')){
			
			$('#modal_error').modal('show');
			setTimeout(function(){
				$('#modal_error').modal('hide');
			}, 1000);
			return;
		}
		
		var params = {};
		params["auth"] = this.getCredentials();
		params["newUsername"] = $('#input_user_newUsername').val();
		params["newPassword"] = $('#input_user_newPassword').val();

		var that = this;
		this.jsonRPC.call('authenticator.changeUsernameAndPassword', params).then(function (result) {

			if(result["state"] == 0) {
				//new login required
				that.internalLogout();

				$('#modal_success').modal('show');
				setTimeout(function(){
					$('#modal_success').modal('hide');
				}, 1000);
			}else{
				$('#modal_error').modal('show');
				setTimeout(function(){
					$('#modal_error').modal('hide');
				}, 1000);
			}
		});
	}

	logout() {
		var params = {};
		params["auth"] = this.getCredentials();

		var that = this;
		this.jsonRPC.call('authenticator.logout', params).then(function (result) {
			that.internalLogout();
			$('#modal_user_successText').html("<strong>Success!</strong> You are logged out!");
			$('#modal_user_success').modal('show');
			setTimeout(function(){
				$('#modal_user_success').modal('hide');
			}, 1000);
		});
	}

	internalLogout(){
		this.username = "";
		this.apiToken = "";
		this.authLevel = 0;
		$('#div_user_login').show();
		$('#div_user_main').hide();
		$('#btn_span_user').html("Login");
		//change color of login button
		$('#user-tab').removeClass("login-true");
		$('#user-tab').addClass("login-false");
	}

	getCredentials(){
		var params = {};
		params["username"] = this.username;
		params["apiToken"] = this.token;
		return params;
	}

	getUsername(){
		return this.username;
	}

	getToken(){
		return this.token;
	}

	getAuthLevel(){
		/* Dear Hacker, this documentation is only for me an it will give you no chance to access. */
		// 0 = ANY
		// 7 = USER
		// 15 = ADMIN - try to manipulate this. Good luck :P
		return this.authLevel;
	}

	isLoggedIn(){
		return ((this.username != "") && (this.token != ""));
	}

	handleError(state){
		if(state == 4) this.logout();
	}
}