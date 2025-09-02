
Copy required .service files to /etc/systemd/system/

Test with:

sudo systemctl start romi-camera.service
sudo systemctl status romi-camera.service
sudo systemctl stop romi-camera.service
sudo systemctl status romi-camera.service

Enable on start-up:

sudo systemctl enable romi-camera.service

