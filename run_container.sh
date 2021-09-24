#!/bin/bash

readonly SCRIPT_NAME=${0##*/}

print_help() {
    cat <<END
Usage: $SCRIPT_NAME [OPTION]... COMMAND
  COMMAND:
    start         start docker container
    stop          stop docker container
    restart       restart docker container

  OPTION:
    -h            show this help message and exit
END
    exit 2
}

print_error() {
    cat <<END 1>&2
$SCRIPT_NAME: $1
Try -h option for more information
END
}

if [[ $1 != -* ]]; then
    cmd=$1
    shift 1
fi

while getopts :s:h option; do
    case "$option" in
    h)
        print_help
        exit 0
        ;;
    :)
        print_error "option requires an argument -- '$OPTARG'"
        exit 1
        ;;
    \?)
        print_error "unrecognized option -- '$OPTARG'"
        exit 1
        ;;
    esac
done

shift $((OPTIND - 1))

if [[ -z $cmd ]]; then
  if [ $# -ge 1 ]; then
      cmd=$1
  else
      cmd="start"
  fi
fi

start_container () {
    docker run -d --rm -it -e DISPLAY=$DISPLAY -v /tmp/.X11-unix/:/tmp/.X11-unix \
	   --mount type=bind,source="$(pwd)/sses",target=/source/sses            \
	   --mount type=bind,source="$(pwd)/demo",target=/source/demo            \
   	   --mount type=bind,source="$(pwd)/stdsc",target=/source/stdsc          \
	   --mount type=bind,source="$(pwd)/settings",target=/source/settings    \
	   --name sses sses
}

stop_container () {
    docker stop sses
}

if [ $cmd == "start" ]; then
    start_container
elif [ $cmd == "stop" ]; then
    stop_container
elif [ $cmd == "restart" ]; then
    stop_container
    start_container
fi

