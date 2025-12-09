class About {
    constructor(jsonRPC, auth, lang, home) {

        this.lang = lang;
        this.jsonRPC = jsonRPC;
        this.auth = auth;
        this.home = home;
    }
    run() {
        var that = this;
        $('#tab_about').load('html/about.html #content', function () {
            // after html loaded:

            that.setLanguage(that.lang);
            that.refresh();
        });
    }

    setLanguage(lang) {
        this.lang = lang;
        if (lang == "de") {
            $('.lang-de').show();
            $('.lang-en').hide();
        } else {
            $('.lang-de').hide();
            $('.lang-en').show();
        }
    }

    disableSet() {

    }

    enableSet() {

    }

    refresh() {
        
    }

}