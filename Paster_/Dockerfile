FROM ubuntu:latest

RUN apt-get update && apt-get install -y \
    build-essential \
    make \
    g++ \
    curl \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . /app

CMD ["make", "build"]
