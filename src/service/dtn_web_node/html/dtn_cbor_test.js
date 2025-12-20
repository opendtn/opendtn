/*
 * 	dtn_cbor_test.js
 * 	Author: Markus Töpfer
 *
 *      ------------------------------------------------------------------------
 *
 *      #GLOBAL Variables
 *
 *      ------------------------------------------------------------------------
 */

var dtn_cbor_test = {

    block_counter: 0,

    init: function(){

        this.add_primary_block();
        this.add_payload_block();

    },

    create_uuid: function() {
        function s4() {
            return Math.floor((1 + Math.random()) * 0x10000).toString(16).substring(1);
        }
        return s4() + s4() + "-" + s4() + "-" + s4() + "-" + s4() + "-" + s4() + s4() + s4();
    },

    add_primary_block: function(){

        console.log("adding primary block");
        var root = document.getElementById('cbor');
        var primary = document.createElement("div");
        primary.id = "primary";
        root.appendChild(primary);
        primary_tag = document.createElement("p");
        primary_tag.innerHTML="Primary Block";
        primary.appendChild(primary_tag);

        var version = document.createElement("div");
        version.width = "100%";
        primary.appendChild(version);
        var version_item = document.createElement("input");
        version_item.id = "primary_version";
        version_item.value = "0x07";
        version_item.style.float  = "left";
        var version_tag = document.createElement("p");
        version_tag.innerHTML="version";
        version.appendChild(version_item);
        version.appendChild(version_tag);

        var flags = document.createElement("div");
        flags.width = "100%";
        primary.appendChild(flags);
        var flags_item = document.createElement("input");
        flags_item.id = "primary_flags";
        flags_item.value = "0x00";
        flags_item.style.float  = "left";
        var flags_tag = document.createElement("p");
        flags_tag.innerHTML="flags";
        flags.appendChild(flags_item);
        flags.appendChild(flags_tag);

        var crc_type = document.createElement("div");
        crc_type.width = "100%";
        primary.appendChild(crc_type);
        var crc_type_item = document.createElement("input");
        crc_type_item.id = "primary_crc_type";
        crc_type_item.value = "0x00";
        crc_type_item.style.float  = "left";
        var crc_type_tag = document.createElement("p");
        crc_type_tag.innerHTML="crc_type";
        crc_type.appendChild(crc_type_item);
        crc_type.appendChild(crc_type_tag);

        var dest = document.createElement("div");
        dest.width = "100%";
        primary.appendChild(dest);
        var dest_item = document.createElement("input");
        dest_item.id = "primary_dest";
        dest_item.value = "destination";
        dest_item.style.float  = "left";
        var dest_tag = document.createElement("p");
        dest_tag.innerHTML="destination";
        dest.appendChild(dest_item);
        dest.appendChild(dest_tag);

        var source = document.createElement("div");
        source.width = "100%";
        primary.appendChild(source);
        var source_item = document.createElement("input");
        source_item.id = "primary_source";
        source_item.value = "source";
        source_item.style.float  = "left";
        var source_tag = document.createElement("p");
        source_tag.innerHTML="source";
        source.appendChild(source_item);
        source.appendChild(source_tag);

        var report = document.createElement("div");
        report.width = "100%";
        primary.appendChild(report);
        var report_item = document.createElement("input");
        report_item.id = "primary_report";
        report_item.value = "report";
        report_item.style.float  = "left";
        var report_tag = document.createElement("p");
        report_tag.innerHTML="report";
        report.appendChild(report_item);
        report.appendChild(report_tag);

        var timestamp = document.createElement("div");
        timestamp.width = "100%";
        primary.appendChild(timestamp);
        var timestamp_item = document.createElement("input");
        timestamp_item.id = "primary_timestamp";
        timestamp_item.value = "0x00";
        timestamp_item.style.float  = "left";
        var timestamp_tag = document.createElement("p");
        timestamp_tag.innerHTML="timestamp";
        timestamp.appendChild(timestamp_item);
        timestamp.appendChild(timestamp_tag);

        var sequence = document.createElement("div");
        sequence.width = "100%";
        primary.appendChild(sequence);
        var sequence_item = document.createElement("input");
        sequence_item.id = "primary_sequence";
        sequence_item.value = "0x00";
        sequence_item.style.float  = "left";
        var sequence_tag = document.createElement("p");
        sequence_tag.innerHTML="sequence number";
        sequence.appendChild(sequence_item);
        sequence.appendChild(sequence_tag);

        var lifetime = document.createElement("div");
        lifetime.width = "100%";
        primary.appendChild(lifetime);
        var lifetime_item = document.createElement("input");
        lifetime_item.id = "primary_lifetime";
        lifetime_item.value = "0x00";
        lifetime_item.style.float  = "left";
        var lifetime_tag = document.createElement("p");
        lifetime_tag.innerHTML="lifetime";
        lifetime.appendChild(lifetime_item);
        lifetime.appendChild(lifetime_tag);

        var fragment = document.createElement("div");
        fragment.width = "100%";
        primary.appendChild(fragment);
        var fragment_item = document.createElement("input");
        fragment_item.id = "primary_fragment";
        fragment_item.value = "0x00";
        fragment_item.style.float  = "left";
        var fragment_tag = document.createElement("p");
        fragment_tag.innerHTML="fragment length";
        fragment.appendChild(fragment_item);
        fragment.appendChild(fragment_tag);

        var total = document.createElement("div");
        total.width = "100%";
        primary.appendChild(total);
        var total_item = document.createElement("input");
        total_item.id = "primary_total";
        total_item.value = "0x00";
        total_item.style.float  = "left";
        var total_tag = document.createElement("p");
        total_tag.innerHTML="total data length";
        total.appendChild(total_item);
        total.appendChild(total_tag);

        var crc = document.createElement("div");
        crc.width = "100%";
        primary.appendChild(crc);
        var crc_item = document.createElement("input");
        crc_item.id = "primary_crc";
        crc_item.value = "0x0000";
        crc_item.style.float  = "left";
        var crc_tag = document.createElement("p");
        crc_tag.innerHTML="crc (4 or 8 byte value)";
        crc.appendChild(crc_item);
        crc.appendChild(crc_tag);
    },

    add_payload_block: function(){

        console.log("adding payload block");

        var root = document.getElementById('cbor');

        var delimiter = document.createElement("hr");
        root.appendChild(delimiter);

        var blocks = document.createElement("div");
        blocks.id = "blocks";
        root.appendChild(blocks); 

        var block = document.createElement("div");
        block.id = "payload";
        blocks.appendChild(block);

        var block_tag = document.createElement("p");
        block_tag.innerHTML="Payload Block";
        block.appendChild(block_tag);

        var code = document.createElement("div");
        code.width = "100%";
        block.appendChild(code);
        var code_item = document.createElement("input");
        code_item.id = "payload_code";
        code_item.value = "0x01";
        code_item.style.float  = "left";
        var code_tag = document.createElement("p");
        code_tag.innerHTML="code";
        code.appendChild(code_item);
        code.appendChild(code_tag);

        var num = document.createElement("div");
        num.width = "100%";
        block.appendChild(num);
        var num_item = document.createElement("input");
        num_item.id = "payload_num";
        num_item.value = "0x01";
        num_item.style.float  = "left";
        var num_tag = document.createElement("p");
        num_tag.innerHTML="number";
        num.appendChild(num_item);
        num.appendChild(num_tag);

        var flags = document.createElement("div");
        flags.width = "100%";
        block.appendChild(flags);
        var flags_item = document.createElement("input");
        flags_item.id = "payload_flags";
        flags_item.value = "0x00";
        flags_item.style.float  = "left";
        var flags_tag = document.createElement("p");
        flags_tag.innerHTML="flags";
        flags.appendChild(flags_item);
        flags.appendChild(flags_tag);

        var crc_type = document.createElement("div");
        crc_type.width = "100%";
        block.appendChild(crc_type);
        var crc_type_item = document.createElement("input");
        crc_type_item.id = "payload_crc_type";
        crc_type_item.value = "0x00";
        crc_type_item.style.float  = "left";
        var crc_type_tag = document.createElement("p");
        crc_type_tag.innerHTML="crc_type";
        crc_type.appendChild(crc_type_item);
        crc_type.appendChild(crc_type_tag);

        var data = document.createElement("div");
        data.width = "100%";
        block.appendChild(data);
        var data_item = document.createElement("input");
        data_item.id = "payload_data";
        data_item.value = "some payload data";
        data_item.style.float  = "left";
        var data_tag = document.createElement("p");
        data_tag.innerHTML="data";
        data.appendChild(data_item);
        data.appendChild(data_tag);

        var crc = document.createElement("div");
        crc.width = "100%";
        block.appendChild(crc);
        var crc_item = document.createElement("input");
        crc_item.id = "payload_crc";
        crc_item.value = "0x0000";
        crc_item.style.float  = "left";
        var crc_tag = document.createElement("p");
        crc_tag.innerHTML="crc (4 or 8 byte value)";
        crc.appendChild(crc_item);
        crc.appendChild(crc_tag);
    },

    add_block: function(){

        console.log("adding block");
        this.block_counter++;

        var delimiter = document.createElement("hr");
        
        var blocks = document.getElementById("blocks");
        blocks.appendChild(delimiter);
        var block = document.createElement("div");
        block.id = "data" + this.block_counter;
        blocks.appendChild(block);

        var block_tag = document.createElement("p");
        block_tag.innerHTML="Data Block";
        block.appendChild(block_tag);

        var code = document.createElement("div");
        code.width = "100%";
        block.appendChild(code);
        var code_item = document.createElement("input");
        code_item.id = "code" + this.block_counter;
        code_item.value = "0x01";
        code_item.style.float  = "left";
        var code_tag = document.createElement("p");
        code_tag.innerHTML="code";
        code.appendChild(code_item);
        code.appendChild(code_tag);

        var num = document.createElement("div");
        num.width = "100%";
        block.appendChild(num);
        var num_item = document.createElement("input");
        num_item.id = "num" + this.block_counter;
        num_item.value = "0x01";
        num_item.style.float  = "left";
        var num_tag = document.createElement("p");
        num_tag.innerHTML="number";
        num.appendChild(num_item);
        num.appendChild(num_tag);

        var flags = document.createElement("div");
        flags.width = "100%";
        block.appendChild(flags);
        var flags_item = document.createElement("input");
        flags_item.id = "flags" + this.block_counter;
        flags_item.value = "0x00";
        flags_item.style.float  = "left";
        var flags_tag = document.createElement("p");
        flags_tag.innerHTML="flags";
        flags.appendChild(flags_item);
        flags.appendChild(flags_tag);

        var crc_type = document.createElement("div");
        crc_type.width = "100%";
        block.appendChild(crc_type);
        var crc_type_item = document.createElement("input");
        crc_type_item.id = "crc_type" + this.block_counter;
        crc_type_item.value = "0x00";
        crc_type_item.style.float  = "left";
        var crc_type_tag = document.createElement("p");
        crc_type_tag.innerHTML="crc_type";
        crc_type.appendChild(crc_type_item);
        crc_type.appendChild(crc_type_tag);

        var data = document.createElement("div");
        data.width = "100%";
        block.appendChild(data);
        var data_item = document.createElement("input");
        data_item.id = "data" + this.block_counter;
        data_item.value = "some payload data";
        data_item.style.float  = "left";
        var data_tag = document.createElement("p");
        data_tag.innerHTML="data";
        data.appendChild(data_item);
        data.appendChild(data_tag);

        var crc = document.createElement("div");
        crc.width = "100%";
        block.appendChild(crc);
        var crc_item = document.createElement("input");
        crc_item.id = "crc"+ this.block_counter;
        crc_item.value = "0x0000";
        crc_item.style.float  = "left";
        var crc_tag = document.createElement("p");
        crc_tag.innerHTML="crc (4 or 8 byte value)";
        crc.appendChild(crc_item);
        crc.appendChild(crc_tag);

    },

    generateJSON: function(){

        let request = {
            uuid : this.create_uuid(),
            event: "bundle_send_request",
            parameter: {}
        }

        request.parameter.socket = {};
        request.parameter.socket.host = document.getElementById("ip").value;
        request.parameter.socket.port = parseInt(document.getElementById("port").value);

        request.parameter.primary = {};
        request.parameter.primary.version = document.getElementById("primary_version").value;
        request.parameter.primary.flags = document.getElementById("primary_flags").value;
        request.parameter.primary.crc_type = document.getElementById("primary_crc_type").value;
        request.parameter.primary.dest = document.getElementById("primary_dest").value;
        request.parameter.primary.source = document.getElementById("primary_source").value;
        request.parameter.primary.report = document.getElementById("primary_report").value;
        request.parameter.primary.timestamp = document.getElementById("primary_timestamp").value;
        request.parameter.primary.sequence = document.getElementById("primary_sequence").value;
        request.parameter.primary.lifetime = document.getElementById("primary_lifetime").value;
        request.parameter.primary.fragment = document.getElementById("primary_fragment").value;
        request.parameter.primary.total_data = document.getElementById("primary_total").value;
        request.parameter.primary.crc = document.getElementById("primary_crc").value;

        request.parameter.payload = {};
        request.parameter.payload.code = document.getElementById("payload_code").value;
        request.parameter.payload.num = document.getElementById("payload_num").value;
        request.parameter.payload.flags = document.getElementById("payload_flags").value;
        request.parameter.payload.crc_type = document.getElementById("payload_crc_type").value;
        request.parameter.payload.data = document.getElementById("payload_data").value;
        request.parameter.payload.crc = document.getElementById("payload_crc").value;

        if (this.block_counter > 0){

            request.parameter.blocks = new Array(this.block_counter);

            for (let i = 1; i <= this.block_counter; i++){

                request.parameter.blocks[i-1] = {};
                request.parameter.blocks[i-1].code = document.getElementById("code" + i).value;
                request.parameter.blocks[i-1].num = document.getElementById("num" + i).value;
                request.parameter.blocks[i-1].flags = document.getElementById("flags" + i).value;
                request.parameter.blocks[i-1].crc_type = document.getElementById("crc_type" + i).value;
                request.parameter.blocks[i-1].data = document.getElementById("data" + i).value;
                request.parameter.blocks[i-1].crc = document.getElementById("crc" + i).value;
                
            }
        }

        return request;

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

dtn_cbor_test.init();