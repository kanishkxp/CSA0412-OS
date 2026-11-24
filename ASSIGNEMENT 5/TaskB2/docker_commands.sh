#!/usr/bin/env bash
# Commands for Task B2. Run them one by one on a machine with Docker and keep the output.
docker build -t result-portal .
docker images result-portal                       # image size
docker history result-portal                      # layers
docker run -d --name portal -p 8000:8000 result-portal
docker ps
docker exec portal whoami                         # should print: portal
curl http://localhost:8000/
docker rm -f portal
# resource limits and their effect
docker run -d --name portal-lim --cpus 0.5 --memory 256m -p 8000:8000 result-portal
docker stats --no-stream portal-lim
ab -n 5000 -c 50 http://localhost:8000/result/192511278 &
docker stats --no-stream portal-lim
docker rm -f portal-lim
# compose with a database volume
docker compose up -d
docker compose ps
docker volume ls
docker compose down
# smaller image
docker build -f Dockerfile.multistage -t result-portal:multi .
docker images
