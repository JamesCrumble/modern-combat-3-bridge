# MC3 Bridge (Modern Combat 3: Fallen Nation -> Android APK для OnePlus 13s): состояние проекта

Цель: то же, что в `../modern-combat-4-zero-hour-probe` (APK с qemu-user + мост GLES на GPU телефона).
Каркас (portbase, обёртка, qemu, инструменты, тулчейны) скопирован из MC4 и переименован mc4 -> mc3,
слой игры (`port/game/`) переписан под MC3 1.0.1.

Обновлено: 2026-10-21. Версия порта 0.1 (альфа): меню и первая миссия (проверено пользователем), 60 кадров/с на OnePlus 13s (`logs/phone_1.png`).
Как делалось — [docs/PORTING_GUIDE.md](docs/PORTING_GUIDE.md), история версий — [CHANGELOG.md](CHANGELOG.md).

## Правила работы
- По-русски, кратко.
- Глобально ничего не ставить: тулчейны копировать из соседних проектов, бинарники качать в папку проекта.
- Основная версия — 1.0.1. 1.1.7g отложена (решение пользователя 21.10).
- git: origin github.com/JamesCrumble/modern-combat-3-bridge (master), релизы — тэги `vX.Y`, файлы в `release/<версия>/`
  (`tools/make_release.sh`).

## Игра 1.0.1
- `gamebin/Modern+Combat+3+HD+v1.0.1(MaxGraph).apk`, пакет `com.gameloft.android.GAND.GloftM3HP` (versionCode 101),
  модифицированный (в dex есть `com/savegame/SavesRestoringPortable`).
- `gamebin/Modern Combat 3 HD v1.0.1 Remastered.zip` -> внутри `com.gameloft.android.GAND.GloftM3HP.zip` — кэш
  (`files/`: `data/DATA_1..DATA_4_4` 1,3 ГБ, плоские `.bdae/.tga/.glsl`, `*.sobfs`, `contextData.dump0`). OBB нет.
- Библиотека `lib/armeabi-v7a/libModernCombat3.so` (9,2 МБ, SHA1 `cc1af20a9a42e5bd84aec4e96267be3ba447b417`),
  **стрипнута**: имён движка нет, только JNI и libc-подобные экспорты. Звук через Java `AudioTrack`.

### Порядок запуска (из dex)
`JNI_OnLoad` -> `GL2JNILib.init` (движок сам зовёт `setupPaths`/`createView`) -> `MyVideoView.Init` ->
`GloftM3HP.Init` (метод экземпляра) -> `MyVideoView.playMovie` -> `GloftM3HP.nativeMC3Init` -> `MyVideoView.SetFinished`
(плеера нет) -> `initGL` -> `getViewSettings` -> `stateChanged(true)` -> `resize` -> `step` в цикле, кадр показывается после каждого `step`.
`setPaths(resource, home, temp)`: ресурсы = `/sdcard/Android/data/<пакет>/files` -> `<game>/files`.

### Что понадобилось (по порядку)
1. `dladdr` не было в переходниках — добавлен (`portbase/thunks/libc/misc.cpp`).
2. GloftDRM: `nativeMC3Init` зовёт `GloftM3HP.d/da/db/dc`; `db` = `res/raw/igli.bin`, `dc` = `res/raw/serialkey.txt`.
   Обёртка распаковывает `res/raw` из APK в `<game>/res/raw`.
3. abort в dlmalloc игры (`create_mspace` на 30 МБ): `sysconf(0x28)` — в bionic это `_SC_PAGE_SIZE`, в glibc другое.
   `sysconf` теперь переводит номера bionic (`misc.cpp`).

4. Миссия не грузилась (чёрный экран, звук меню): брифинг `data/briefing/Briefing_M01.mp4` запускается через
   `new Intent(activity, MyVideoView)` + `startActivity`, движок ждёт `MyVideoView.SetFinished`. Добавлены фейковые
   `android/content/Intent` и `android/app/Activity`; ролик пропускается (`SetFinished` на следующем кадре, `mc3_video_tick`).
5. Деньги (`tweaks.cpp`, по смещениям с проверкой слов): синие кредиты — поле `профиль+0x90`; `+0xceeac` геттер,
   `+0xcf884` начисление, `+0xd028c` списание, `+0x7c644` объект игры (`+0x27c` локальный профиль). Найдено по строке
   `NbCreditsLocal`. `MC3_MONEY_INFINITE` / `MC3_MONEY_RESTORE` в настройках, раздел «Деньги (эксперимент)».
   Бесконечные синие кредиты проверены пользователем (9 999 999). Золото (правый верхний угол) не меняется: в профиле
   его нет (поля профиля: кредиты +0x90, заработано всего +0xa4, опыт +0x34, звание +0x38, статистика +0x4c..+0x77),
   вероятно, оно магазинное/серверное (Gameloft Exchange). Пользователь: возможно, не понадобится.

## Что есть в папке
| путь | что |
|---|---|
| `port/game/` | слой игры MC3 (`main.cpp`, `io_paths.cpp`, `jni/gl2jni_activity.cpp`, `jni/*_gen.inc` из `tools/gen_stubs.sh`) |
| `mc3data/v101/` | для ПК: `lib/`, `assets/`, `res/raw/`, `game.apk`, `files/` (кэш) |
| `extracted/v101`, `extracted/v117g` | распакованные APK, `classes.txt`, `strings.txt`, `imports/exports.txt`; `extracted/classes.txt`/`strings.txt` — ссылки на v101 |

Телефон: `tools/build_install.sh` (сборка+установка), `tools/push_data.sh` (APK + кэш, 1,6 ГБ), `tools/launch.sh`, `tools/pull_log.sh`.
На телефоне: `Android/data/com.jamescrumble.mc3/files/` — `mc3-1.0.1.apk` и `com.gameloft.android.GAND.GloftM3HP/files/`.
Обёртка также принимает zip кэша рядом с APK (распаковывает `files/` один раз).

## Не проверено / не сделано
- Дальше первой миссии, сохранения, пауза и возврат в приложение, геймпад.
- Ролики (логотип, брифинги) не проигрываются — плеера нет, сразу `SetFinished`.
- `tweaks.cpp`, `input_bridge.cpp` остались от MC4 (символы MC4 не находятся — безвредно); геймпад не настроен.
- «Качество графики» (`MC3_GENERATION` -> `useGeneration0/1/2`) на картинке не проверено.
- Вложенный zip кэша («Remastered» с APK внутри) и извлечение APK из него собраны, на телефоне не проверены
  (на телефоне кэш залит распакованным).
- 1.1.7g (`ANMP.GloftM3HM`, OBB main.1120 + patch.11428) — разбор отложен.

## Золото: что проверено (поиск остановлен 21.10)
- Все функции профиля помечены тегом (`vtable[12](тег)` + шум `lrand48`): `stor` — кредиты (`+0xceeac` геттер, `+0xcf884`
  начисление, `+0xd028c` списание), `lifp` — опыт `+0x34` (`+0xcee50`), звание `+0x38` (`+0xcedbc`), `stps` — статистика
  (24-битные поля в байтах `+0x6c..+0x77`, например `+0xcef08`, `+0xcefe0`). Поле `+0x94` — указатель на битовый массив
  (открытые предметы), `+0xa4` — кредиты, заработанные за всё время.
- Меню: `NbCreditsLocal` = геттер кредитов на `[объект игры+0x27c]`, `NbCreditsOnline` — он же на `[+0x280]`
  (оба после патча показывают 9 999 999, но золото в углу — не они).
- Строки золота только в магазине/трекинге: `Gold Credits|Blue Credits`, `virtual_cash_type` (`+0xc4a28` — сортировка
  каталога Gameloft Exchange, `data/MC3.json`, `MC3Store`), трекинг `&events[%i][data][coins]` (`+0xd715c`, значение
  из поля события `+36`, флаг `+40`: cash/coins). `+0xd7590` — таблица цен, не баланс.
- Куда смотреть дальше, если понадобится: кто заполняет событие трекинга с флагом coins, код Gameloft Exchange
  (`GameloftExchange::RefreshProductList`), обработчик кнопки «+» у золота.
