# qrcli
CLI utility to convert any input to a QR code

## Usage
```
qrcli -help
qrcli 'encoded text'
qrcli -correction 3 'encoded text'
```

## Build
```
gcc nob.c -o nob
./nob
# Outputs: ./qrcli
```
