# gems

Лабораторная по С++. Игра "Gems 3 в ряд".

## Сборка

```
git clone https://github.com/krasnikovra/gems
```

Линковка SFML - динамическая. В корне есть скрипты копирующие все dll в нужную директорию после сборки, но VS генерирует их Read-Only. Поэтому при первой сборке может выдастся ошибка наподобие

```
1>C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Microsoft\VC\v170\Microsoft.CppCommon.targets(156,5): error MSB3073: выход из команды ".\dllscpy-d.bat
1>C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Microsoft\VC\v170\Microsoft.CppCommon.targets(156,5): error MSB3073: :VCEnd" с кодом 1.
```

Зайдите в свойства директории `Debug` (`Release`) и уберите флажок "Только для чтения". Тогда все dll будут автоматически копироваться скриптом.
