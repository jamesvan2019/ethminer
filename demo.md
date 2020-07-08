# 验证过程 示例

### 公钥 key.pem （**这个公钥是每出厂一个tpm矿机将此公钥提交至矿池或者节点备案 最好与地址或者有个编号对应便于校验**）

    -----BEGIN PUBLIC KEY-----
    MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAufkAAU1DStwa7tm1a6UZ
    C42pgXPxTH6uX/aJim7PG/NoCHeEd8bkwvxPwrJjy9tp1dgDYxEugg9Mrsbu2scA
    N/w9JA0o/+mfLFvKgHLyccTR+8J8rq1z+aTuVDQ38mk9yfOPiHXebCu5vA6bJJXt
    ZgV2mYsPcovGe+IgZZp12z8nUv/I9Ur9cruEjjvhaiFip1J4ykJGPsBeSuwxhVfa
    dfwdXUSR3sR/JJSmXqKcPEV8+wwcJAmchitfddxbES6PvdpCHR8aKwEpw+yGkFxb
    7FuOZ1Q51wH7F3giX5ocITuj3rEFejImeqelSWBVKLQyI/5QU4AbIlxY8an/iaNl
    5wIDAQAB
    -----END PUBLIC KEY-----


### 提交的 mixhash 

    0x41fb67790aa63142404e4c7cc4d1a8d04255f68e6b0d600070a6502eb3facb83
### 提交的 nonce 

    0x04472efeb70741ae
### 签名的输入源 datain.txt (mixhash+nonce)

    0x41fb67790aa63142404e4c7cc4d1a8d04255f68e6b0d600070a6502eb3facb830x04472efeb70741ae
### 提交的 signature.hex

    0x14000b00000134a5ed82d9644bb6db8b3f8ef0ba11c5cbd4ec387978b7dc509625bb7d4374d159edde88203909bb0382ff33456d369239f9a0a3d0ca96e46ad08d26a710d474e5031b65351a5d929fe600955d5361c447601ce2a8f7cf751d9890d679fad0ecca348d2e4868bca1690501117e039ef59ed0e0db86367563bf258c05de711b34f2385d9b3879b7dade5d18502ac214eca2a8f682285efd58e26f693132e312d6bf6b1bbd85b52a0c6403877c2d719797a61a453254a5a3922e806770d381e3d93c88add009614ec6cb1f431b16843fdd95455c3623bc7d3729a69c6fbe2137808b94048a9f1ce6d767b7263e0a2360d60767852fbd1e6951fd7daf4e0367c591

### 0x开头可去掉 转换成 binary 到 signature.bin 文件中
```golang
//golang
b,_ := hex.DecodeString("14000b00000134a5ed82d9644bb6db8b3f8ef0ba11c5cbd4ec387978b7dc509625bb7d4374d159edde88203909bb0382ff33456d369239f9a0a3d0ca96e46ad08d26a710d474e5031b65351a5d929fe600955d5361c447601ce2a8f7cf751d9890d679fad0ecca348d2e4868bca1690501117e039ef59ed0e0db86367563bf258c05de711b34f2385d9b3879b7dade5d18502ac214eca2a8f682285efd58e26f693132e312d6bf6b1bbd85b52a0c6403877c2d719797a61a453254a5a3922e806770d381e3d93c88add009614ec6cb1f431b16843fdd95455c3623bc7d3729a69c6fbe2137808b94048a9f1ce6d767b7263e0a2360d60767852fbd1e6951fd7daf4e0367c591")
ioutil.WriteFile("signature.bin", b, 777)
```
### 验证
```bash
$ dd if=signature.bin of=signature.raw bs=1 skip=6 count=256

$ openssl dgst -verify key.pem -keyform pem -sha256 -signature signature.raw datain.txt

$ Verified OK
```