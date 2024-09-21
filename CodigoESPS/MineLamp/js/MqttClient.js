const utils = require('./utils');
const mqtt = require('mqtt');

class MqttClient {
    constructor(options, topicsSubscribed) {
        utils.assert(typeof(options)==='object', '"options" must be an object');
        utils.assert(typeof(topicsSubscribed)==='string' || (Array.isArray(topicsSubscribed)&&topicsSubscribed.every(x=>typeof(x)==='string')), '"topicSubscribed" must be a string or an array of strings');
        this.client = mqtt.connect(options);
        for(let ev of ['connect', 'error', 'message'])
            this.client.on(ev, (topic, message) => { this['on'+ev](topic, message); });
        
        if(Array.isArray(topicsSubscribed))
            for(const topic of topicsSubscribed)
                this.client.subscribe(topic);
        else
            this.client.subscribe(topicSubscribed);
    }

    onconnect() {
        console.log('Conectado!');
    }

    onerror(err) {
        throw new Error(err);
    }

    onmessage(topic, message) {
        console.log(`Mensagem recebida: <${topic}> ${message.toString()}`);
    }

    publish(topic, message) {
        utils.assert(typeof(topic)==='string', '"topic" must be a string');
        utils.assert(typeof(message)==='string', '"message" must be a string');
        this.client.publish(topic, message);
    }
}

module.exports = MqttClient;
