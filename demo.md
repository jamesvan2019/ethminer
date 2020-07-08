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


### 提交的 headerhash (pool下发的)

    0x58220f1988d26ab18fd501d8416a4808d1ffafd48dc558dc1d64f44eec501dbc
### 提交的 nonce 

    0xa422420917e3ab70
### 签名的输入源 datain.txt (headerhash+nonce)

    0x58220f1988d26ab18fd501d8416a4808d1ffafd48dc558dc1d64f44eec501dbc0xa422420917e3ab70
### 提交的 signature.hex

    0x14000b0000014e3e389d374f5fac0ee32661c8d23d36ca6dbfee20ba69ea7fd58562900bfc4adbeebae5cb8f1f823a1ff6c29f3e1bbb857200f0c66851bc94bea31d0e9d18e5a9f4c4fc2eaf9912ee402fb4b260b06cf8a3ffaef73c6816412c88b1561e7bf995c70e768f5df5126b704d17c549ca4c1fbad87edc7ce4c5be1b938355ca5f4aefc66172acedb9b8edbd76e6e26f7f335a3dbb8d6dfebbbe9e0b52a943b63403923985f319e44e8cb3c56868cb9125cb33f92208d91620c7e37fa0ac0118017c4374f2c6792b2f790480f506ba456ff6834ae2b772b4278e0e033562b161fd964eb199ce7e4109553a25f385f10f5e2f9e4eb45080eb3f8230ec6db4a42cc9bf

### 0x开头可去掉 转换成 binary 到 signature.bin 文件中
```golang
//golang
b,_ := hex.DecodeString("14000b0000014e3e389d374f5fac0ee32661c8d23d36ca6dbfee20ba69ea7fd58562900bfc4adbeebae5cb8f1f823a1ff6c29f3e1bbb857200f0c66851bc94bea31d0e9d18e5a9f4c4fc2eaf9912ee402fb4b260b06cf8a3ffaef73c6816412c88b1561e7bf995c70e768f5df5126b704d17c549ca4c1fbad87edc7ce4c5be1b938355ca5f4aefc66172acedb9b8edbd76e6e26f7f335a3dbb8d6dfebbbe9e0b52a943b63403923985f319e44e8cb3c56868cb9125cb33f92208d91620c7e37fa0ac0118017c4374f2c6792b2f790480f506ba456ff6834ae2b772b4278e0e033562b161fd964eb199ce7e4109553a25f385f10f5e2f9e4eb45080eb3f8230ec6db4a42cc9bf")
ioutil.WriteFile("signature.bin", b, 777)
```
### 验证
```bash
$ dd if=signature.bin of=signature.raw bs=1 skip=6 count=256

$ openssl dgst -verify key.pem -keyform pem -sha256 -signature signature.raw datain.txt

$ Verified OK
```