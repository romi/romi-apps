var cameraMountController = null;
var mountControlPanel = null;

class Homing
{
    constructor(mode, order, speed) {
        this.mode = mode,
        this.order = order;
        this.speed = speed;
    }
}

class Range
{
    constructor(min, max) {
        this.min = min,
        this.max = max;
    }

    length() {
        return this.max - this.min;
    }
}

class Axis
{
    constructor(data) {
        this.name = data.name;
        this.type = data.type;
        if (data.homing) {
            this.homing = new Homing(data.homing.mode,
                                     data.homing.order,
                                     data.homing.speed);
        }
        if (data.range) {
            this.range = new Range(data.range[0], data.range[1]);
        }
    }  
}

class CNC
{
    constructor(id, controller, viewer) {
        this.id = id;
        this.controller = controller;
        this.viewer = viewer;
        viewer.setCNC(this); // FIME
        this.axes = [];
        this.position = [0, 0, 0];
        this.controller.callWhenConnected(this);
        this.is_powered = false;
    }

    getId() {
        return this.id;
    }

    connected() {
        console.log('CNC.connected');
        this.initAxesAndPosition();
    }  

    initAxesAndPosition() {
        this.requestAxes();
    }

    requestAxes() {
        this.controller.invoke(this, 'cnc-get-axes');
    }

    requestPosition() {
        this.controller.invoke(this, 'cnc-get-position');
    }

    moveto(x, y, z, speed) {
	console.log("moveto " + x + " " + y + " " + z);
        this.position.x = x;
        this.position.y = y;
        this.position.z = z;
        var params = { 'x': this.position.x,
                       'y': this.position.y,
                       'z': this.position.z,
                       'speed': speed };
        this.controller.invoke(this, 'cnc-moveto', params);
    }

    handleErrorMessage(error) {
        console.log('CNC: Error: ' + error.message);
    }

    handleTextMessage(response) {
        if (response.method == "cnc-get-axes") {
            this.setAxes(response.result);
            this.viewer.update(this);
        } else if (response.method == "cnc-get-position") {
            this.setPosition(response.result);
            this.viewer.update(this);
        } else {
            console.log('CNC: Unknown method: ' + response.method);
        }
    }  

    handleBinaryMessage(buffer) {
        console.log('CNC: Not expected');
    }

    setAxes(result) {
        console.log("TODO: CNC.setAxis: result=" + JSON.stringify(result));
        for (let i = 0; i < result.length; i++) {
            let axis = new Axis(result[i]);
            if (axis.range.length() > 0) { 
                this.axes.push(axis);
            }
        }
    }  

    setPosition(result) {
        this.position.x = result['x'];
        this.position.y = result['y'];
        this.position.z = result['z'];
        console.log('Position: ' + result);
    }  
}  

class CNCViewer
{
    constructor() {
        this.cnc = undefined;
        this.root = undefined;
        this.element = undefined;
        this.posView = [];
    }

    attach(root) {
        this.root = root;
        if (this.element) {
            this.root.appendChild(this.element);
        }
    }

    setCNC(cnc) {
        this.cnc = cnc; // FIXME: I don't like this...
    }
    
    update(cnc) {
        console.log('CNCViewer: update');
        if (!this.element) {
            this.makeView(cnc);
        } else {
            this.updateView(cnc);
        }
    }

    updateView(cnc) {
        this.updatePosition(cnc);
    }
    
    updatePosition(cnc) {
        for (let i = 0; i < cnc.axes.length; i++) {
            this.posView[i].setValue(cnc.position[i]);
        }
    }
    
    makeView(cnc) {
        this.element = document.createElement("div");
        this.element.className = "cnc";

        let pos = this.makePositionView(cnc);
        this.element.appendChild(pos);

        let buttons = this.makeButtonBar(cnc);
        this.element.appendChild(buttons);
        
        if (this.root) {
            this.root.appendChild(this.element);
        }
    }
    
    callMoveto() {
        console.log('CNCViewer: callMoveto');
        var pos = [0, 0, 0];
        for (let i = 0; i < this.posView.length; i++) {
            console.log('CNCViewer: ' + this.posView);
            console.log('CNCViewer: ' + this.posView[i]);
            console.log('CNCViewer: ' + this.posView[i].getValue());
            console.log('CNCViewer: ' + parseFloat(this.posView[i].getValue()));
            pos[i] = parseFloat(this.posView[i].getValue());
        }
        console.log("moveto (" + pos[0] + "," + pos[1] + "," + pos[2] + ")");
        if (this.cnc) {
            this.cnc.moveto(pos[0], pos[1], pos[2], 0.3);
        }
    }

    callHoming() {
        console.log('CNCViewer: callHoming');
    }
    
    makePositionView(cnc) {
        console.log('CNCViewer: makePositionView');
        var element = document.createElement("div");
        element.className = "position";

        for (let i = 0; i < cnc.axes.length; i++) {
            let e = this.makeAxisView(cnc, i);
            element.appendChild(e);
        }
        
        var button = new Button((e) => { this.callMoveto(); }, "", 'Move to');
        element.appendChild(button.element);
        return element;
    }
    
    makeAxisView(cnc, index) {
        let axis = cnc.axes[index];
        var element = document.createElement('div');
        element.className = 'position-xyz-section';

        var text = document.createElement('span');
        text.className = 'position-label';
        let s = axis.name + ": ";
        text.innerHTML = s;
        element.appendChild(text);        

        let textfield = this.makeTextField(cnc.position[index]);
        this.posView.push(textfield);
        element.appendChild(textfield.element);        
        
        text = document.createElement('span');
        text.className = 'position-label';
        if (axis.type == "linear") {
            s = "m ";
        } else if (axis.type == "angular") {
            s = "°";
        }
        if (axis.range) {
            s += " in range [" + axis.range.min + ", " + axis.range.max + "]";
            element.appendChild(text);        
        }
        text.innerHTML = s;
        element.appendChild(text);        
        
        return element;
    }
        
    makeTextField(value) {
        return new TextField((target) => { let x = parseFloat(target.value);
                                           this.callMoveto(); },
                             'position-value', value, 4);
    }

    makeButtonBar(cnc) {
        console.log('CNCViewer: makePositionView');
        var element = document.createElement("div");
        element.className = "button-panel";
        
        var button = new Button((e) => { this.callHoming(); }, "", 'Homing');
        element.appendChild(button.element);
        return element;
    }
}

function initCNC(name, controller, createControlPanel)
{
    let viewer = new CNCViewer('cnc');
    viewer.attach(document.getElementById('position-app'));
    cnc = new CNC(name, controller, viewer);
    if (createControlPanel) {
        //controlPanel = new CNCControlPanel(cnc);
    }
}
