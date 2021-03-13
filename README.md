### Experiment for Meiji System on Nifty Cloud

#### Preemptive Knowledge

- mask: a representation of age(0-122) and gender(m/f/o)
```
mask = age + 5 (gender == 'm')
mask = 128 + age + 5 (gender == 'f')
mask = 256 + age + 5 (gender == 'o')
```

#### Structure

##### executables
Server Side
- update.cpp
1. Usage: `update [filename of encrypted mask] [number of medicine] [List of medicines] [number of side effect] [List of side effects]`
2. How it works?
    1. It will move the encrypted mask into the meiji-nifty library.
    2. It will update the `med.inv` and `side.inv`: inverted index for medicines and side effects.
    3. It will generate auxiliary information for new record.

- server.cpp
1. Usage: `server [PORT] ([number of thread] default: 28)`
2. How it works?
    1. Listen to PORT until a user replies.
    2. Receive [(Encrypted) query mask] [number of query medicine] [List of query medicines] [number of query side effects] [List of query side effects] from client.
    3. Filter by `query medicines` and `query side effects` using **merge of inverted index**
    4. Split the filtered result into chunks of (100, 500, 1000, 2000), extract the ciphertext `mask` from files. Put them into slots. each chunk one `Ctxt`. Multithreading begins.
    5. Subtract with [(Encrypted) query mask] got in step 2.
    6. Broaden the range from `-5` to `+5`, get a `vector<Ctxt>` of 11 elements.
    7. Use a pyramidal way to do `Π` (this step will greatly consume level): `11` -> `6` -> `3` -> `2` -> `1`
    8. The result is timed with a random integer within `1 - 256`.
    9. The result is returned back to the user, sepearted with chunks.
    10. Receive the user's reply of which record(s) the user want.
    11. Give user the auxiliary information of the wanted records.
    12. Session closed and wait for another user to link.

Client Side
- client.cpp
1. Usage: `client [IP Address] [PORT] [age] [gender] [list of query medicines] [query side effects]`
2. How it works?
    1. Try to connect to `[IP Address]:[PORT]`.
    2. Receive [age] [gender] [number of query medicine] [List of query medicines] [number of query side effects] [List of query side effects] from user.
    3. Encrypted [age] [gender] into [(Encrypted) query mask].
    4. Send [(Encrypted) query mask] [number of query medicine] [List of query medicines] [number of query side effects] [List of query side effects] to server.
    5. Receive the result of each chunk sent by server.
    6. Decrypt the result. find 0's inside, and tell server `pair<chunk id, position id>` is desired.
    7. Receive the auxiliary data from server and output to user. Close.

God Side
- keygen.cpp
1. Usage: `keygen`
2. Generate FHE context and keys.

- data_enc.cpp
1. Usage: `data_enc`
2. How does it work?
    1. From the generated dummy data, initialize the inverted index `med.inv` and `side.inv`.
    2. Call `update` for each plaintext record with encrypted [mask].

##### files
- auxdata
1. `0-39999.bin`: auxiliary information (non-query related information)
2. `med.inv` and `side.inv`: inverted index for the medicine and side effects
- encdata
1. `0-39999.bin`: encrypted mask for each records
- settings
1. `context.bin`: FHE context
2. `pk.bin`: FHE public key
3. `sk.bin`: FHE secret key (Only live in Server side)
4. `contextsetting.txt`: Instruct how keygen set the parameters like `m`, `p`, `r`, `L`, etc.
- dmydata
1. `data.csv`: generated by project [meiji-dummy-data](https://fs.yama.info.waseda.ac.jp/amadeus/meiji-dummy-data)
