/*
 * 	dtn_echo_test.js
 * 	Author: Markus Töpfer
 *
 *
 *      This JS file expects some ov_websocket with a configure function,
 *      to set itself as protocol for the websocket to consume incoming messages.
 *
 *      ------------------------------------------------------------------------
 *
 *      #GLOBAL Variables
 *
 *      ------------------------------------------------------------------------
 */

var dtn_echo_test = {

    debug: true,
    session: null,
    client: null,

    create_uuid: function() {
        function s4() {
            return Math.floor((1 + Math.random()) * 0x10000).toString(16).substring(1);
        }
        return s4() + s4() + "-" + s4() + "-" + s4() + "-" + s4() + "-" + s4() + s4() + s4();
    },

    init: function(){

        if (null == dtn_websocket){

            console.error("dtn_echo_test is a protocol for dtn_websocket " +
                "some WEBSOCKET MUST be present!");

        } else {

            console.log("dtn_websocket.incoming set as dtn_echo_test.incoming.")
            dtn_websocket.incoming = dtn_echo_test.incoming;
            dtn_websocket.debug(this.debug);

            dtn_websocket.connect(window.location.hostname);

        }

        this.client = this.create_uuid();



    },

    incoming: function(msg){

        /* Expect some JSON message input. */

        if (true == this.debug)
            console.log("<-- RECV "+ JSON.stringify(msg, null, 2));


        switch (msg.event) {

            case "echo":
                console.log("ECHO RECEIVED.")
                break;

            default:
                
                break;

        }

    },

    echo : function(msg){

        let request = {
            uuid : this.create_uuid(),
            event: "echo",
            parameter: {}
        }

        request.parameter.data = msg;
        dtn_websocket.send(JSON.stringify(request));

    },

    shutdown : function(){

        let request = {
            uuid : this.create_uuid(),
            event: "shutdown",
            parameter: {}
        }
        dtn_websocket.send(JSON.stringify(request));

    }


};

/*
 *      ------------------------------------------------------------------------
 *
 *      GENERIC INIT IN TEMPLATE
 *
 *      This JS file expects some ov_websocket with a configure function,
 *      to set itself as protocl for the websocket to consume incoming messages.
 *
 *      ------------------------------------------------------------------------
 */

dtn_echo_test.init();