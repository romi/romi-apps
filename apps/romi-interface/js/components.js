
class Selector
{
    constructor(callback, classname, options) {
        this.callback = callback;
        this.element = document.createElement('select');
        this.element.className = classname;
        this.element.addEventListener('change', (e) => { this.updateValue(e); });
        this.makeOptions(options);
        this.value = null;
    }

    makeOptions(options) {
        for (const option of options) {
            let element = document.createElement('option');
            element.value = option.value;
            element.label = option.label;
            if (option.selected) {
                element.selected = true;
                this.value = option.value;
            }
            this.element.appendChild(element);
        }
    }

    updateValue(e) {
        this.value = e.target.value;
        this.callback(this);
    }
}

class TextField
{
    constructor(callback, classname, value, length) {
        this.callback = callback;
        this.value = value;
        this.element = document.createElement('input');
        this.element.type = 'text';
        this.element.value = value;
        this.element.size = length;
        this.element.className = classname;
        this.element.addEventListener('change', (e) => { this.updateValue(e); });
    }

    updateValue(e) {
        this.value = e.target.value;
        this.callback(this);
    }
}

class Checkbox
{
    constructor(callback, classname, value) {
        this.callback = callback;
        this.element = document.createElement('input');
        this.element.type = 'checkbox';
        this.element.checked = value;
        this.element.className = classname;
        this.element.addEventListener('change', (e) => { this.updateValue(e); });
        this.value = value;
    }

    updateValue(e) {
        this.value = e.target.checked;
        this.callback(this);
    }
}

class Grid
{
    constructor(root, components, columns) {
        this.root = root;
        this.components = components;
        this.columns = columns;
        this.buildIt();
    }

    buildIt() {
        let count = 0;
        var container = document.createElement('div');
        container.className = 'container';
        while (count < this.components.length) {
            this.buildRow(container, count);
            count += this.columns;
        }
        this.root.appendChild(container);
    }

    buildRow(container, start) {
        var row = document.createElement('div');
        row.className = 'row';
        for (let i = 0; (i < this.columns) && (start + i < this.components.length); i++) {
            this.buildColumn(row, start + i);
        }
        container.appendChild(row);
    }

    buildColumn(row, index) {
        var col = document.createElement('div');
        col.className = 'col index-app';

        var header = document.createElement('div');
        header.className = 'index-header';

        var a = document.createElement('a');
        a.href = this.components[index].href;
        a.innerHTML = this.components[index].name;
        header.appendChild(a);
        col.appendChild(header);
        
        var body = document.createElement('div');
        this.components[index].viewer.attach(body);
        col.appendChild(body);
        row.appendChild(col);
    }

}
