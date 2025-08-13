

```csv
key,type,encoding,value
settings,namespace,,
hwVersion,data,u32,12      -- версия платы v11 (20,28л), v12 (35л)
scrBright,data,u16,100
h1_max_pow,data,i32,75     -- Мощность в %
t1_clbr_fct,data,i32,-20   -- Корректировка темп-ры в x10. (т.е. -20 = -2.0)
```




ESP-IDF terminal
```
python $env:IDF_PATH/components/nvs_flash/nvs_partition_generator/nvs_partition_gen.py generate "nvs/csv/nvs_T-3_P75_(20,28L)_v11.csv" "nvs/nvs_T-3_P75_(20,28L)_v11.bin" 0x5000

python $env:IDF_PATH/components/nvs_flash/nvs_partition_generator/nvs_partition_gen.py generate "nvs/csv/nvs_T-4_P75_(37L)_v12.csv" "nvs/nvs_T-4_P75_(37L)_v12.bin" 0x5000

```


