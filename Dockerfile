FROM alpine:latest
RUN apk update
RUN apk add readline readline-dev gcc libc-dev linux-headers make
# For test
RUN apk add mandoc man-pages tar-doc
COPY . /home/tsh/
