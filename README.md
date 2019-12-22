# download

> University project for FEUP, MIEIC (RCOM).

A simple FTP client that downloads a file from an FTP server.

## Building

Run `make` in the root of the project. The `download` executable will be built inside a `bin` directory.

## Usage

The application expects the following usage:

```
download <user> <password> <host> <url_path>
```

These arguments are used to form the connection string: `ftp://[user[:password]@]host/url-path`.

The file pointed to by `url-path` will be downloaded into the Downloads folder, i.e. `~/Downloads/`.
