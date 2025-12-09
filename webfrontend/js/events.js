class Events {
    constructor(jsonRPC, auth, lang) {

        this.lang = lang;
        this.jsonRPC = jsonRPC;
        this.auth = auth;

        this.eventList = null;
        this.eventListString = null;
        this.numNewEvents = 10;
    }
    run() {
        var that = this;
        $('#tab_events').load('html/events.html #content', function () {
            // after html loaded:
            that.setState(10);

            // load list of events from file
            $.getJSON("assets/events.json", function (json) {
                that.eventList = json;
            });

            // on reset button click
            $('#btn_event_reset').click(function () {
                var params = {};
                params["auth"] = that.auth.getCredentials();
                that.jsonRPC.call('event.resetState', params).then(function (result) {
                    if(result["ret"] == 0){
                        $('#modal_success_text_en').html("Success");
				        $('#modal_success_text_de').html("Erfolgreich");
				        $('#modal_success').modal('show');
                        //hide success after 1 second
                        setTimeout(function(){
                            $('#modal_success').modal('hide');
                        }, 1000);
                    }else{
                        $('#modal_error_text_en').html("Something went wrong!");
				        $('#modal_error_text_de').html("Etwas ist schief gegangen!");
				        $('#modal_error').modal('show');
                        //hide success after 1 second
                        setTimeout(function(){
                            $('#modal_error').modal('hide');
                        }, 1000);
                    }
                    that.refresh();
                });
            });

            // on clear button click
            $('#btn_event_clear').click(function () {
                var params = {};
                params["auth"] = that.auth.getCredentials();
                that.jsonRPC.call('event.clear', params).then(function (result) {
                    if(result["ret"] == 0){
                        $('#modal_success_text_en').html("Success");
				        $('#modal_success_text_de').html("Erfolgreich");
				        $('#modal_success').modal('show');
                        //hide success after 1 second
                        setTimeout(function(){
                            $('#modal_success').modal('hide');
                        }, 1000);
                    }else{
                        $('#modal_error_text_en').html("Something went wrong!");
				        $('#modal_error_text_de').html("Etwas ist schief gegangen!");
				        $('#modal_error').modal('show');
                        //hide success after 1 second
                        setTimeout(function(){
                            $('#modal_error').modal('hide');
                        }, 1000);
                    }
                    that.refresh();
                });
            });

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
        var that = this;

        // do not show delete button, if user is now admin
        if(this.auth.getAuthLevel() < 15){
            $('#btn_event_clear').hide();
        }else{
            $('#btn_event_clear').show();
        }

        // get system status
		that.jsonRPC.call('event.getState').then(function (result) {
			that.setState(result["code"]);
		});

        // get a system log file from storage
        $.ajax({
            type: "GET",
            url: "log/system.log",
            dataType: "text",
            success: function (response) {
                // transform csv to array
                var eventLog = [];
                var lines = response.split('\n');
                for (var x in lines) {
                    eventLog[x] = lines[x].split(',');
                }
                that.parse(eventLog);
            }
        });
    }

    parse(eventLog) {
        let table = "";
        this.eventListString = [["Timestamp", " Message",  "Level"]];
        this.numNewEvents = 0;

        var lastReset = this.getLastReset(eventLog);

        // all lines in eventLog
        for (let x in eventLog) {
            // get event
            let timestamp = this.getTimestamp(eventLog[x][0])
            let event = this.getEvent(eventLog[x][1]);
            let state = this.getState(eventLog[x][2]);

            // do not show acknowledged entry's for default users
            if(this.auth.getAuthLevel() < 15){
                // show only new elements
                if(lastReset != null){ // we have acknowledged entry's
                    if (parseInt(x) < parseInt(lastReset)) { // this was an acknowledged entry +1 (just show the acknowledge message)
                        continue; // skip this
                    }
                }
            }

            // start table body
            var row = "<tr>";
            row += "<th scope='row'>" + (parseInt(x)+1) + "</th>"; // add pseudo id
            
            if (event == null) {
                // event unknown
                row += "<td></td> <td></td> <td></td> <td></td> <td></td>"
                continue;
            }

            //push event to string list
            this.eventListString.push([timestamp, event.msg_en, state]);
            
            // put event info to table
            row += "<td>" + timestamp + "</td>";
            row += "<td class='event_log_" + state + "'></td>";
            row += "<td>";
            row += "<span class='lang-de'>" + event.msg_de + "</span>";
            row += "<span class='lang-en'>" + event.msg_en + "</span>";
            row += "</td>";

            // after reset event, everything is true
            // one single reset event is the default state
            if(lastReset == null){
                row += "<td>" + $('#event-badge-new').html() + "</td>";
                this.numNewEvents++;
            }else{
                if (parseInt(x) > parseInt(lastReset)) {
                    row += "<td>" + $('#event-badge-new').html() + "</td>";
                    this.numNewEvents++;
                }else{
                    row += "<td></td>"; //empty
                }
            }
            

            // table end
            row += "</tr>";
            table = row + table; //put element at front - means sort from new to old
        }

        // draw table
        $('#event_table').html(table);

        // move templates in place
        $('.event_log_info').html($('#event-badge-info').html());
        $('.event_log_warning').html($('#event-badge-warning').html());
        $('.event_log_error').html($('#event-badge-fail').html());

        // set event indicator in menu
        if(this.numNewEvents > 0){
            $('#tab_events_btn_badge').html(this.numNewEvents);
        }else{
            $('#tab_events_btn_badge').html(''); 
        }

        this.setLanguage(this.lang); //refresh
    }

    getLastReset(eventLog){
        var i = null;
        for(var x in eventLog){
            // this is a reset with id 0?
            if(eventLog[x][1] == 1000){
                i = x;
            }
        }
        return i;
    }

    getEvent(id) {
        // looking for given event id in loockuptable
        for (let x in this.eventList) {
            if (this.eventList[x].id == id) return this.eventList[x];
        }
        return null;
    }

    getTimestamp(unix_timestamp) {
        // get time object
        let time = new Date(unix_timestamp * 1000);
        //add leading zeros
        var month = "0" + (time.getMonth() + 1);
        var day = "0" + time.getDate();
        var hours = "0" + time.getHours();
        var minutes = "0" + time.getMinutes();
        var seconds = "0" + time.getSeconds();
        //return and trim to 2 charaters
        return time.getFullYear() + "-" + month.slice(-2) + "-" + day.slice(-2) + " " + hours.slice(-2) + ":" + minutes.slice(-2) + ":" + seconds.slice(-2);
    }

    getState(state){
        /*
        enum class State{
            RUNNING = 0,
            INFO = 20,
            WARNING = 50,
            ERROR = 90,
        };*/
        if (state == 0)  return ""; //hide
        if (state == 20) return "info";
        if (state == 50) return "warning";
        if (state == 90) return "error";
        return "";
    }

    setState(state){
        if (state <= 20) $('#event_system_status').html("<h3>"+ $('#event-badge-success').html() +"</h3>");
        if (state == 50) $('#event_system_status').html("<h3>"+ $('#event-badge-warning').html() +"</h3>");
        if (state == 90) $('#event_system_status').html("<h3>"+ $('#event-badge-fail').html() +"</h3>");
    }
}