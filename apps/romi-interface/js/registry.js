

class Registry
{
    constructor(registryIP) {
        this.registryIP = registryIP;
    }
    
    list(response_handler, error_handler) {
        this.sendRequest({ 'request': 'list' }, response_handler, error_handler);
    }
    
    get(topic, response_handler, error_handler) {
        this.sendRequest({ 'request': 'get', 'topic': topic },
                         response_handler, error_handler);
    }

    sendRequest(request, response_handler, error_handler) {
        console.log("registry " + this.registryIP);
        var registrySocket = new WebSocket('ws://' + this.registryIP + ':10101');

        registrySocket.onopen = (event) => {
            console.log(request);
            registrySocket.send(JSON.stringify(request));
        };
        
        registrySocket.onmessage = (event) => {
            console.log(event.data);
            var reply = JSON.parse(event.data);
            if (reply.success) {
                registrySocket.close();
                response_handler(reply);
            } else {
                error_handler();
            }
        }
    }
}
