/*
 * 	dtn_web_node.js
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

var dtn_web_node = {

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

            console.error("dtn_web_node is a protocol for dtn_websocket " +
                "some WEBSOCKET MUST be present!");

        } else {

            console.log("dtn_websocket.incoming set as dtn_web_node.incoming.")
            dtn_websocket.incoming = dtn_web_node.incoming;
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

    login : function(msg){

        let request = {
            uuid : this.create_uuid(),
            event: "login",
            parameter: {}
        }

        request.parameter.password = msg;
        dtn_websocket.send(JSON.stringify(request));

    },

    send: function(){

        console.log("sending CBOR blocks");

        let request = dtn_cbor_test.generateJSON();
        dtn_websocket.send(JSON.stringify(request));
        
    },


 


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

dtn_web_node.init();