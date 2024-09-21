const MqttClient = require('./MqttClient');
const optionsComp = require('../json/optionsComp.json');

const topics = ['display', 'foto', 'lamp1', 'lamp2', 'luz', 'chuva'];
const socketsByTopic={};
for(const topic of topics)
    socketsByTopic[topic]=[];

const client = new MqttClient(optionsComp, topics);

client.onmessage = (topic, message) => {
    message=String(message);
    if(!topics.includes(topic))
        return;
    console.log(`Mensagem recebida <${topic}>: "${message}"`);
    for(socket of socketsByTopic[topic])
        socket.send(message);
}

// Código adaptado de: <https://masteringjs.io/tutorials/express/websockets>
const express = require('express');
const ws = require('ws');

const app = express();

const wsServer = new ws.Server({ noServer: true });
wsServer.on('connection', socket => {
    console.log('Conexão estabelecida!');

    socket.on('message', message => {
        words=String(message).split(',');
        switch(words[0]) {
            case 'topic':
                for(let i=1;i<words.length;i++) {
                    if(topics.includes(words[i]) && !socketsByTopic[words[i]].includes(socket)) {
                        socketsByTopic[words[i]].push(socket);
                        if(!socket.topics)
                            socket.topics=[];
                        socket.topics.push(words[i]);
                        console.log(`Um novo socket foi incluido no tópico "${words[i]}"`);
                    } else if(topics.includes(words[i]))
                        console.log(`Alguém tentou se reinscrever no tópico "${words[i]}"`);
                    else
                        console.log(`Alguém tentou se inscrever no tópico inválido "${words[i]}"`);
                }
                break;
            case 'send':
                if(!words.length)
                    console.log("Alguém tentou enviar algo?");
                else if(words.length===0)
                    console.log(`Alguém tentou enviar nada no tópico "${words[0]}"`);
                else
                    client.publish(words[1], words.slice(2).join(','));
                break;
            default:
                console.log(`Alguém tentou realizar o comando inválido: "${words[0]}"`);
                break;
        }
    });

    socket.on('close', message => {
        console.log('Alguém se desconectou!');
        if(socket.topics)
            socket.topics.forEach(topic => {
                socketsByTopic[topic].splice(socketsByTopic[topic].indexOf(socket), 1);
                console.log(`Alguém se desconectou do tópico "${topic}"`);
            });
    });
});

const server = app.listen(3000);
server.on('upgrade', (request, socket, head) => {
    wsServer.handleUpgrade(request, socket, head, socket => {
        wsServer.emit('connection', socket, request);
    });
});
console.log('Servidor criado!\nEntre em: ws://localhost:3000');
