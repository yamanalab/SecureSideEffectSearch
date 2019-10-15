#!/bin/sh

docker-compose -f ../docker-compose.yml up -d server
docker-compose -f ../docker-compose.yml up client
