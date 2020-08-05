# [BrainFlow Website](https://brainflow.ai/)

### Setup Docker Container For Development

```
cd brainflow
git checkout gh-pages
sudo docker build -t brainflow-site $PWD
sudo docker run -d -p 4000:4000 --name brainflow-site -v $PWD:/srv/jekyll brainflow-site
# open http://0.0.0.0:4000/ to check website
# each time you change config file restart container
sudo docker restart brainflow-site
```