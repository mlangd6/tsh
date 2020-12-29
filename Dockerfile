FROM alpine:latest
RUN apk update
RUN apk add readline readline-dev gcc libc-dev linux-headers make
# For tests
RUN apk add mandoc man-pages tar-doc tar
COPY . /home/tsh/
