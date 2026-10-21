# Как переносилась Modern Combat 3: разбор по шагам

Этот файл — запись того, как игра была доведена от «APK и архив кэша» до кампании на телефоне, в виде
руководства для следующего такого переноса. Каждый шаг описан одинаково: что было видно, как нашли причину,
что и где исправили. Команды приведены так, чтобы их можно было повторить.

Состояние на 2026-10-21: меню и кампания, 60 кадров/с на OnePlus 13s. Короткая сводка — в [README.md](../README.md),
текущее состояние и список недоделанного — в [PROJECT_STATE.md](../PROJECT_STATE.md).

## С чего начинать

Каркас (qemu, APK-обёртка, GL-мост, загрузчик, тулчейны) взят целиком из MC4 Bridge
(`../modern-combat-4-zero-hour-probe`) и переименован (`mc4` → `mc3`, `MC4_` → `MC3_`). Переписан слой игры в
`port/game/` и подготовка данных в `GameProcess.java`.

Любопытная деталь: `portbase` в своё время вырос из PortMaster-порта Modern Combat 3 автора EapRules (следы — в
комментариях `symtab_glprobe.cpp`, `nova3probe/port/harness/verify.sh`). Тот порт делался под версию из Google Play
(`ANMP.GloftM3HM`, OBB), а здесь — 1.0.1 (`GAND.GloftM3HP`, плоский кэш), поэтому слой игры всё равно пишется заново.

Разбор игры — те же команды, что в соседних проектах, результаты лежат в `extracted/v101` (и `extracted/v117g`):

```sh
unzip -l gamebin/<игра>.apk | grep -v res/
readelf -hd lib.so | grep -E 'NEEDED|SONAME'
readelf --dyn-syms -W lib.so | awk '$7=="UND"{print $8}' | sort > extracted/v101/imports.txt
android-sdk/build-tools/34.0.0/dexdump -d classes.dex > extracted/v101/classes.txt
strings -n 6 lib.so > extracted/v101/strings.txt
```

`extracted/classes.txt` и `extracted/strings.txt` — ссылки на `v101`: их читают `tools/dexmethod.py` и
`tools/gen_java_stubs.py`.

## Чем MC3 1.0.1 отличается от MC4

| что | MC4 1.2.3e | MC3 1.0.1 |
|---|---|---|
| пакет | `ANMP.GloftM4HM` | `GAND.GloftM3HP` (Gameloft-магазин, не Google Play) |
| символы | экспортированы C++-имена движка | **стрипнута**: только JNI и libc-подобные экспорты |
| данные | два OBB, движок читает их сам | кэш `files/`: паки `data/DATA_1…DATA_4_4` + плоские файлы |
| свои точки входа | `SUtils/Device.nativeInit`, `MC4.nativeMC4Init` | `GloftM3HP.Init` (метод экземпляра), `nativeMC3Init`, `video/MyVideoView.*` |
| `setPaths` | от движка через `setupPaths` | так же, 3 строки (ресурсы, дом, временная) |
| показ кадра | движок сам (`eglSwapBuffers`) | GLSurfaceView после каждого `step` |
| DRM | четыре `lockPointer` | GloftDRM: файлы `res/raw` из APK (шаг 2) |
| звук | Java `AudioTrack` | так же |
| ролики | `SUtils.playVideo` | `Intent` + `startActivity(MyVideoView)` (шаг 4) |

Порядок запуска прочитан из dex (`tools/dexmethod.py GloftM3HP.GloftM3HP '^(a|onCreate)$' --calls`):

`JNI_OnLoad` → `GL2JNILib.init` (движок сам зовёт `setupPaths`, `createView`) → `MyVideoView.Init` →
`GloftM3HP.Init` → `MyVideoView.playMovie` → `GloftM3HP.nativeMC3Init` → `initGL` → `getViewSettings` →
`stateChanged(true)` (`onSurfaceCreated`) → `resize` (`onSurfaceChanged`) → `step` в цикле.

`Device.nativeInit` экспортирован, но Java его не вызывает — порт тоже. Коды касаний те же, что у всей семьи:
`touchEvent(действие, x, y, палец)`, 1 — вниз, 2 — движение, 0 — вверх.

Какие Java-методы ищет библиотека: имена методов из dex пересекаются со строками библиотеки (так работает
`tools/gen_stubs.sh` → `port/game/jni/*_gen.inc`). Нужные по-настоящему написаны руками в
[gl2jni_activity.cpp](../port/game/jni/gl2jni_activity.cpp): `setupPaths`, `getResource`, `getSDFolder`,
`useGeneration0/1/2`, DRM-методы, `Intent`/`Activity`.

## Как искать причину без символов

У стрипнутой библиотеки `tools/dis.sh` подписывает адреса ближайшим экспортом (`Java_…_init+0x1872c`) —
это бессмысленно, смотреть надо на сам код. Что помогает:

1. **Узнать функцию по коду.** `sysconf` → проверка степени двойки → `time() ^ 0x55555555` — это `init_mparams`
   из dlmalloc, даже без имени (шаг 3).
2. **Ссылки на строки.** `tools/xref_pcrel.py <lib> <строка>` находит `ldr rX,[pc,#…]; add rX,pc,rX` на строку.
   Так нашлись деньги (раздел «Деньги»).
3. **Профиль гостя на телефоне.** Строка `QEMU_GUEST_PROF` в логе: адрес, где стоит поток. Адреса > `text_base`
   (строка `so_load_module returned (text_base=…)`) — код игры, адреса около `libc addrs:` — glibc в sysroot.
4. **Счётчики моста.** `[gl] frames …, 906 cmds, per frame: 1=1 43=1 44=1` — кадр из одной очистки: игра жива,
   но ничего не рисует (шаг 4).

## Шаги

### 1. Не хватает `dladdr`

- **Видно:** `unresolved symbol: dladdr` на ПК при загрузке.
- **Исправление:** `dladdr_impl` в [misc.cpp](../port/portbase/thunks/libc/misc.cpp): адрес внутри кода игры —
  ответ «это модуль игры» с его базой, иначе 0. Свой `Dl_info` из четырёх указателей (раскладка bionic).

### 2. GloftDRM в `nativeMC3Init`

- **Видно:** `Class GloftM3HP does not have static method d()V`, затем `db()[B`, `dc()[B`, `da()Landroid/app/Activity;`
  и SIGSEGV по адресу 8.
- **Как нашли:** в dex эти методы есть (`tools/dexmethod.py GloftM3HP.GloftM3HP '^(d|da|db|dc)$'`):
  `db` читает raw-ресурс `0x7f040002`, `dc` — `GloftDRM.a()`, то есть ресурс `0x7f040005`. Имена ресурсов —
  `aapt dump resources`: `raw/igli` (`igli.bin`, 256 КБ) и `raw/serialkey` (`serialkey.txt`).
- **Исправление:** методы отдают эти файлы; обёртка распаковывает `res/raw/` из APK игры в `<game>/res/raw`
  (`GameProcess.extractPrefix`). Код игры не правится.

### 3. abort в первом `step`: `sysconf`

- **Видно:** `Guest called abort!` сразу после двух `fopen` файла `contextData.dump0` (30 МБ нулей).
- **Как нашли:** стек ведёт в `+0x6c59b4`: `sysconf(40)`, проверка «результат — степень двойки», иначе `abort`.
  Это dlmalloc игры, который строит свою кучу (`create_mspace` на 30 МБ) и спрашивает размер страницы.
- **Причина:** у bionic `_SC_PAGE_SIZE` = 0x28, а переходник отдавал номер в glibc как есть (`THUNK_DIRECT`), где
  40 — совсем другой параметр.
- **Исправление:** `sysconf_impl` переводит номера bionic в glibc (`misc.cpp`, `impl_tab.h`). Неизвестный номер — -1 и
  строка в логе.
- **Правило:** константы `sysconf`, `fcntl`, `ioctl`, `socket` у bionic и glibc не обязаны совпадать; прямой
  проброс таких функций — источник тихих ошибок.

### 4. Кампания: чёрный экран со звуком меню

- **Видно:** меню работает, после выбора миссии чёрный экран, музыка меню играет, падения нет. В логе
  `[gl] … 906 cmds`, файлы больше не открываются.
- **Ложный след:** перед этим в логе `ShowAlert` — это кнопка «+» у кредитов (магазин без сети), не причина.
- **Как нашли:** в кэше есть `data/briefing/Briefing_M01.mp4`. В dex `video.MyVideoView` читает из `Intent` строку
  `video_name` и по окончании зовёт нативный `SetFinished`. В строках библиотеки рядом лежат `android/content/Intent`,
  `putExtra`, `startActivity`: брифинг запускается так же, как логотип при старте. Класса `Intent` в фейковом JNI
  не было — ролик не стартовал, движок ждал `SetFinished` вечно.
- **Исправление:** фейковые `android/content/Intent` (конструктор, `putExtra`, `setFlags`…) и `android/app/Activity`
  (`startActivity`) в `gl2jni_activity.cpp`. `startActivity` запоминает ролик, `mc3_video_tick` на следующем кадре
  вызывает `MyVideoView.SetFinished`. Плеера нет — ролик пропускается.
- **Правило:** «чёрный экран, звук есть, кадры идут» у игр Gameloft — почти всегда ожидание Java (ролик, диалог).
  Смотреть, что игра просила у Java последним.

### 5. Данные на телефоне

- Обёртка ищет APK игры и кэш (`com.gameloft.android.GAND.GloftM3HP/files` с `data/DATA_1`) в папке приложения;
  `game/files` — симлинк на кэш. Zip-архив кэша распаковывается один раз, в том числе вложенный (архив
  «Remastered» несёт внутри `com.gameloft.android.GAND.GloftM3HP.zip` и APK игры — APK берётся оттуда же).
- Сохранения: игра пишет в `/data/data/<пакет игры>`, `port_fix_path` отображает это в `game/saves`.
- `tools/push_data.sh` заливает APK и распакованный кэш (1,6 ГБ) и открывает папки приложению (`chmod 2777`).

### 6. Иконка

У иконки игры 72×72 своя нарисованная белая скруглённая рамка, лаунчер показывал её внутри своей маски.
[icon_fit.py](../port/android/tools/icon_fit.py) (чистый Python + zlib, без PIL) срезает 8 % с каждого края и
масштабирует до 432×432 при сборке APK.

## Деньги без символов

Задача — повторить «бесконечные деньги» MC4, где правились экспорты `PlayerProfileBase::GetTotalCredits`.

1. Строки меню: `NbCreditsLocal`, `NbCreditsOnline`. `tools/xref_pcrel.py` → код по `+0x1e9418`.
2. Значение для UI = `+0x1a33a0(x, 0, 0)` — это форматирование числа («5,000»), а `x` = `+0xceeac(профиль)`.
3. `+0xceeac`: `vtable[12]('stor')`, шум `lrand48` в глобальные переменные, `return [this+0x90]` — поле лежит
   открыто. Профиль = `[объект игры+0x27c]`, объект игры возвращает `+0x7c644`.
4. Рядом с тем же тегом `stor`: `+0xcf884` прибавляет к `+0x90` (потолок 99 999 999, ×50 при отладочном флаге
   объекта игры `+0x44`), `+0xd028c` вычитает с ограничением снизу нулём.
5. Правка ([tweaks.cpp](../port/game/tweaks.cpp)): геттер → `ldr r0,[pc,#0]; bx lr; .word 9999999`, списание →
   `bx lr`. Перед записью первые слова функций сверяются с 1.0.1; страница открывается `mprotect` (правило из
   NOVA3: в уже исполнявшийся код писать только после него). «Задать кредиты» пишет в `[профиль+0x90]` через
   300 кадров после появления профиля.

Золото (правый верхний угол) в профиле не нашлось; что проверено — в PROJECT_STATE.md, раздел «Золото».

## Версия 1.1.7g (отложена)

- Пакет `ANMP.GloftM3HM` (Google Play), данные — OBB `main.1120` и `patch.11428` (`nativeTestReadObb`,
  `/sdcard/Android/obb/…`), кэш 1.0.1 к ней не подходит.
- В `assets/_res0` — снимок `/data/data` установленной игры и следы протектора Bangcle (`libsecexe`, `libsecmain`);
  порту это не мешает: Java игры он не исполняет.
- Свои точки входа: `initGDRM`, `isGamePlay`, `nativeNeedShowDisclaimer`, `nativeSetTouchPadDTLeft`.
- Импорты почти те же, что у 1.0.1 (нет `dladdr`, есть `clock`, `fgetc`).

## Что оказалось полезным правилом

1. **Сначала dex, потом догадки.** Порядок запуска, DRM-методы, ролики — всё было в `classes.dex`.
2. **Переходник «напрямую» — тоже ошибка.** Номера констант bionic и glibc различаются.
3. **Чёрный экран со звуком — ожидание Java.**
4. **Стрипнутая библиотека читается через строки UI**: от имени переменной меню до поля в объекте — три шага.
5. **Правка по смещению — только с проверкой исходных слов** и после `mprotect`.
