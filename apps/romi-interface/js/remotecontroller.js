
class RemoteController
{
    constructor(name, registry, remoteAddress) {
        this.name = name;
        this.registry = registry;
        this.remoteAddress = remoteAddress;
        this.socket = null;
        this.connected = false;
        this.connectionHandlers = [];
        this.handlers = {};
        this.binaryHandler = null; // FIXME
        this.encoder = new TextEncoder();
        this.createReconnectDialog();
        this.connect();
    }

    createReconnectDialog() {
        this.dialog = document.createElement('div');
        this.dialog.style.display = "none";
        this.dialog.style.backgroundColor = '#ffffff90';
        this.dialog.style.position = 'fixed';
        this.dialog.style.top = '0';
        this.dialog.style.left = '0';
        this.dialog.style.width = '100%';
        this.dialog.style.height = '100%';
        this.dialog.style.top = '0';
        this.dialog.style.padding = "100px";
        this.dialog.style.textAlign = "center";
        
        let text = document.createElement('p');
        text.innerHTML = "The connection is lost.";
        this.dialog.appendChild(text);
        
        let p = document.createElement('p');
        let button = document.createElement('a');
        button.href = "#.";
        button.innerHTML = "RECONNECT";
        p.appendChild(button);
        this.dialog.appendChild(p);
        button.addEventListener('click', (e) => { this.reconnect()});

        document.body.appendChild(this.dialog);
    }

    showReconnectDialog() {
        this.dialog.style.display = "block";
    }

    hideReconnectDialog() {
        this.dialog.style.display = "none";
    }

    reconnect() {
        this.socket = null;
        this.hideReconnectDialog();
        this.connect();
    }
    
    connect() {
        console.log("connect");
        this.socket = null;
        this.hideReconnectDialog();
        if (this.remoteAddress)
            this.connectToDevice();
        else
            this.findRemoteAddress();
    }

    findRemoteAddress() {
        var self = this;
        this.registry.get(this.name,
                          (data) => {
                              self.remoteAddress = data.address;
                              self.connectToDevice();
                          },
                          () => {
                              console.log("Failed to get the address of " + self.name);
                          });
    }

    connectToDevice() {
        console.log("Opening connection to " + this.remoteAddress);
        this.socket = new WebSocket('ws://' + this.remoteAddress);
        this.socket.addEventListener('message', (event) => {
            this.tryHandleMessage(event.data);
        });
        this.socket.addEventListener('open', (event) => {
            this.ignition();
        });
        this.socket.addEventListener('error', (event) => {
            console.log('WebSocket error: ' + event);
        });
        this.socket.addEventListener('close', (event) => {
            this.handleConnectionLost();
        });
    }
    
    callWhenConnected(handler) {
        if (this.connected) {
            handler.connected();
        } else {
            this.connectionHandlers.push(handler);
        }
    }

    handleConnectionLost() {
        this.showReconnectDialog();
    }

    ignition() {
        this.connected = true;
        for (const handler of this.connectionHandlers) {
            handler.connected();
        }
        this.connectionHandlers = [];
    }

    tryHandleMessage(buffer) {
        try {
            this.handleMessage(buffer);
        } catch (error) {
            console.error(error);
        }
    }

    handleMessage(buffer) {
        if (this.isTextMessage(buffer)) {
            this.handleTextMessage(buffer);
        } else {
            this.handleBinaryMessage(buffer);
        }
    }
    
    isTextMessage(buffer) {
        return (typeof buffer === 'string' && buffer.charAt(0) == '{');
    }

    handleTextMessage(buffer) {
        var response = JSON.parse(buffer);
        if (response.error) {
            this.handleErrorMessage(response);
        } else {
            let handler = this.getHandler(response.id); 
            if (handler) {
                handler.handleTextMessage(response);
            } else {
                console.log('RemoteController: No handler');
            }
        }
    }  

    handleErrorMessage(response) {
        console.log('RemoteController: Method: ' + response.method
                    + ', Error: ' + response.error.message);
        let handler = this.getHandler(response.id); 
        if (handler) {
            handler.handleErrorMessage(response.error);
        } else {
            console.log('RemoteController: No handler');
        }
    }  

    handleBinaryMessage(buffer) {
        if (this.binaryHandler) {
            this.binaryHandler.handleBinaryMessage(buffer);
        } else {
            console.log('RemoteControlle: No handler');
        }
    }  

    getHandler(id) {
        return this.handlers[id];
    }
    
    invoke(obj, method, params) {
        this.handlers[obj.getId()] = obj;
        this._invoke(obj, method, params, false);
    }
    
    invokeBinary(obj, method, params) {
        this.binaryHandler = obj;
        this._invoke(obj, method, params, true);
    }
    
    _invoke(obj, method, params, binary) {
        if (!params)
            params = {};
        this._send({'id': obj.getId(), 'method': method, 'params': params }, binary);
    }
    
    _send(request, binary) {
        if (this.connected) {
            var s = JSON.stringify(request);
            var message = s;
            if (binary)
                message = this.encoder.encode(s)
            this.socket.send(message);
        } else {
            throw 'The RemoteController is not connected!';
        }
    }
}
