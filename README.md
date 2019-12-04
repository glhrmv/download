# download

> University project for FEUP, MIEIC (RCOM).

## Usage

Run `make` in the root of the project. The `download` executable will be built inside a `bin` directory.

The application expects the following usage:

```
./bin/download <user> <password> <host> <url_path>
```

These arguments are used to form the connection string: `ftp://[<user>:<password>@]<host>/<url_path>`.

The file pointed to by `url-path` will be downloaded into the Downloads folder, i.e. `~/Downloads/`.
