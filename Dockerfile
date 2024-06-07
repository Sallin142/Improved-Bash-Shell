# Use the official Ubuntu image as a parent image
FROM ubuntu:latest

# Set the working directory inside the container
WORKDIR /usr/src/app

# Install necessary packages
RUN apt-get update && apt-get install -y \
    build-essential \
    valgrind \
    libreadline-dev

# Copy the current directory contents into the container at /usr/src/app
COPY . .

# Build the shell program
RUN make clean && make

# Run the shell program
CMD ["./shell"]
