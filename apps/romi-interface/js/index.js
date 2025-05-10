
class Index
{
    constructor(registry) {
        var self = this;
        this.registry = registry;
        this.registry.list((data) => { self.handleList(data.list) },
                           () => { self.handleListError(); });
    }

    handleList(list) {
        console.log(list);

        var components = [];
        for (let i = 0; i < list.length; i++) {
            var app = list[i];

            if (app.type == "camera") {
                var controller = new RemoteController(app.topic,
                                                      this.registry,
                                                      app.address);
                let viewer = new ImageViewer(null, 'camera-preview');
                let camera = new RemoteCamera(app.topic, controller, viewer);
                components.push({ 'name': app.topic,
                                  'href': 'camera.html',
                                  'viewer': viewer });
            }
        }
        let root = document.getElementById("index");
        new Grid(root, components, 3);
    }

    handleListError() {
        console.log("Failed to obtain the list of apps");
    }
}
