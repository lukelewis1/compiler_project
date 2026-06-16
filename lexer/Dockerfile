FROM alpine:latest

RUN apk add --no-cache clang make

WORKDIR /app

COPY . .

RUN make

CMD ["./lexer", "tests/test_valid.c"]