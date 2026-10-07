# ReArrakis engineering journal

The Russian development history below is preserved from the source Dune branch.
For a concise English overview, see [status.md](status.md).

## Standalone migration verification — 2026-10-07

The cleaned ReArrakis tree builds the supported USA ROM with 40,348 translated
68000 instructions and 3,095 guarded Z80 variants. All 286 retained engine
checks and four ROM-free Dune adapter checks passed. The synthetic demo and an
installed wheel build/run also passed; the wheel includes the required runtime
headers and ymfm license.

Native game fixtures passed for Z80 RET/NOP patches, projectile dispatch and
creation, encoded references, mouse-only front menus and house choices, native
building menus, and adaptive rendering/input. The view fixture covers 30
size/zoom combinations, cursor/HUD separation, zoomed orders and placement.
A 7,200-frame smoke run ended at PC $00118C after 83,059,078 instructions with
no fault. A separate 20-million-instruction synthesized-audio run completed
with 229,726 YM writes and 1,347,522 stereo frames, without a fault.

The new minimap click path has a synthetic SDL event regression check. Actual
minimap gameplay and hardware-dependent fullscreen/audio remain manual checks.
No full campaign completion is claimed. Source ROM data, generated C, sound
templates, executables and test captures remain local in ignored build/.

---

# Dune: рабочий контекст

Отдельный проект Dune — The Battle for Arrakis для Linux. База RROP:
`291ea812663e69451251b6ab8e6ef50c22015d4a`. RROP развивается отдельно.
Пользовательский ROM USA, 1 МиБ; точный SHA-256 хранится в profiles/dune-us.json.
ROM, извлечённые данные, генерированный C и executable остаются в build/,
исключённом из Git. ROM и извлечённые данные не публикуются.

## Текущий результат

Нативная SDL2-сборка запускается с синтезом YM2612/PSG и исходным управлением.
Переведено 40323 инструкций 68000 и 3095 guarded Z80 variants.
Runtime не декодирует опкоды CPU во время исполнения.

Свежий запуск с настоящим синтезом прошёл 7200 кадров и 83060844 инструкции
68000 без fault: заставка, выбор дома, загрузка и работа первой миссии.
После кадра 3350 проверка прекращает нажатия, чтобы миссия работала без паузы.
Полное прохождение и все команды игрока ещё не проверены.

Отдельный запуск настоящего синтеза на 20 млн инструкций: 229726 YM2612
writes, 672 PSG writes, 1347522 stereo PCM frames, peak 4836, без fault.
15 тестов discovery прошли, включая регрессию чтения таблицы через известный код.
Проверены синтаксис Python/launcher и git diff --check.

## Сборка и запуск

```sh
python3 tools/build_dune.py '/путь/к/Dune - The Battle for Arrakis (U) [!].gen'
./run-dune.sh
```

Стрелки — крестовина, Z/X/C — A/B/C, Enter — Start.
Пробел — пауза runtime, Tab — ускорение, Esc — выход.
Нужны Python 3.10+, C11/C++17, pkg-config, SDL2 development files.

Воспроизводимая проверка без синтеза, но с работающим Z80:

```sh
cc -std=c11 -O0 tools/smoke_dune.c -o build/smoke
./build/smoke 7200 mute
```

Для настоящего синтеза скомпилировать harness с GENESIS_AUDIO и связать
с объектом ymfm через genesis_recompiler.build.build_audio_backend.
Аргумент `on` включает синтез и WAV в build/smoke-audio.wav.

## Особенности профиля

- Таблица switch в $0172A4 содержит 31 смещение, хотя маска индекса
  допускает 32 значения. Лишний элемент — код. Общий анализ теперь
  отклоняет slices, читающие таблицу через известные инструкции.
  Подтверждённые 31 адрес хранятся в профиле.
- Знаковые switches используют EXT.L или SUBQ.L, BMI, CMPI.L, BGT, ADD.L,
  MOVE.W (PC,D0.L), JMP (PC,D0.W). Сборщик проверяет всю последовательность
  и одинаковую ветку выхода, прежде чем добавить цели 0..bound.
- Остальные дополнительные entries подтверждены реальным нативным исполнением.
  Это покрытие наблюдённых путей, а не гарантия полного прохождения.
- Z80 uploads: стартовый ROM $2C4..$2EA и драйвер $2952..$41DC.
  Переведены IRQ $38 и вариант стартового $AF→$E9.
- Драйвер меняет $2B3..$2B7 на 18 26 18 26 C9, затем $2B7..$2B8 на D9 08;
  при синтезе $2C4..$2C5 и $302 становятся NOP. Добавлены guarded images.
  Игра сама загружает и изменяет live RAM; шаблоны не подменяют её состояние.
- Смещения Z80-инструкций $15F9 и $1616 самомодифицируются. Переведены
  все 256 смещений; остальные байты инструкций проверяются runtime.

## Мышь: первый рабочий вариант

Пользователь выбрал мышь для выбора юнитов и команд кликами.
ЛКМ подаёт оригинальную команду A; ПКМ — A при выбранном юните, иначе B;
средняя кнопка — B (отмена). Меню производства и размещение также поддерживают мышь; прокрутка карты — стрелками.

Hooks включаются только для точного SHA-256 поддерживаемого ROM.
Координаты курсора: $FFBF12/$FFBF14, границы: $FFBF1A..$FFBF20.
Сбрасывается предыдущая дельта $FFBF0E, камера и координаты юнитов не меняются.
Клик ждёт обновления native hover tile, затем подаёт исходный pad edge A/B
после VBlank-поллинга $006176/$00617A или $006E02/$006E06. Выбор, проверку приказа и движение исполняет сама игра.
Выбранный юнит определяется по $FFC25C. Рабочий экран проверяется по callback
$FFE002 ($6092/$6D10) и допустимому диапазону границ курсора, включая 0..256/0..160 при размещении. Клавиатура имеет приоритет;
при потере фокуса, паузе и выходе курсора из окна очередь сбрасывается.
SDL logical renderer переводит координаты окна в пиксели оригинального кадра.

В нативной проверке на первой миссии ЛКМ выбрал пехотинца $FF23B0,
ПКМ на другую клетку запустил движение: позиция изменилась с $24801680
на $25601760. Быстрые проверки адаптера и SDL-событий проходят.
С холодного старта mouse smoke прошёл 5200 кадров / 60191105 инструкций без fault;
оба клика обработаны, позиция юнита $24801680 → $258017E0.

Воспроизводимые проверки:

```sh
cc -std=c11 tools/test_dune_mouse.c -o build/test-dune-mouse
./build/test-dune-mouse
cc -std=c11 -DDUNE_MOUSE_SDL_TEST tools/test_dune_mouse.c -o build/test-dune-mouse-sdl $(pkg-config --cflags --libs sdl2)
./build/test-dune-mouse-sdl
cc -std=c11 -O0 -DDUNE_MOUSE_SMOKE tools/smoke_dune.c -o build/smoke-mouse -lm
./build/smoke-mouse 5200 mute
```

Наблюдавшиеся callback/command paths добавлены в профиль, включая $00746C
из живого SDL-запуска. Полное прохождение, все меню и типы приказов не проверены.

Дополнительный адрес $01118E подтверждён остановкой пользовательского запуска.
Это отдельная подпрограмма MOVEM/RTS с прямыми вызовами $02E1D8/$02E184;
её корень добавлен в профиль для статического перевода.

## Проверка базы и громкость

Остановка $029026 воспроизведена двойным ЛКМ по строительной базе.
Её причина — signed switch с SUBQ.L #2,D0 перед BMI/BGT ($028FEA..$029000).
Сборщик теперь распознаёт этот вариант нормализации и переводит всю таблицу
из 17 смещений, включая меню производства. Остальные проверки шаблона сохранены.

Прогнаны 75 сценариев через SDL-обработчик событий и нативное исполнение:
все пары ЛКМ/ПКМ/средней кнопки на базе, четырёх группах юнитов, песке и области
миникарты; быстрые двойные клики и очереди кликов; повторные команды, отмена,
прокрутка; выбор постройки, попытки размещения и FIX. Все завершились без fault.
Проверяется не только отсутствие остановки: двойной ЛКМ должен установить page
$13 и открыть настоящий экран производства. Экран проверен по изображению.
Отдельно проверяются четыре направления в меню и выход (76-й сценарий).
Дополнительно настоящий YM2612/PSG прошёл 5200 кадров / 60328091 инструкцию,
двойной клик по базе открыл page $13 без fault.

Покрыты четыре ветви таблицы навигации $008532..$008538 и четыре ветви
отрисовки $00978E..$00979A; дополнительные наблюдённые пути внесены в профиль.
Полное прохождение и все здания/миссии пока не проверены.

```sh
cc -std=c11 -O0 tools/smoke_dune_mouse.c -o build/smoke-mouse-matrix $(pkg-config --cflags --libs sdl2) -lm
./build/smoke-mouse-matrix
```

По умолчанию громкость SDL-воспроизведения Dune увеличена в 4 раза (+12 dB).
У измеренной записи 86.8 с peak 5381 и средний уровень около -36.4 dBFS;
усиление даёт peak 21524 и около -24.4 dBFS. Мягкое ограничение выше 24000
защищает signed PCM от переполнения при более громких эффектах.
DUNE_VOLUME=0..200 задаёт процент от новой громкости (по умолчанию 100).
Это регулировка воспроизведения; запись WAV сохраняет исходный уровень.
Проверены усиление, нулевой уровень, отрицательные значения PCM, весь диапазон
signed 16-bit и монотонность ограничителя. Другие игры используют прежний звук.

## Установка постройки и PCM: исправление

Пользователь сообщил остановку Z80 $A002CF после установки постройки и отсутствие
мыши в меню/режиме размещения. Драйвер на $13FE..$140B выбирает RET ($C9) либо
NOP ($00) для $02CF по младшему nibble команды. Добавлен guarded voice image
с NOP; runtime продолжает проверять байты инструкций. Тест исполняет сам live
writer для обеих команд и затем обе версии $02CF; декодера опкодов в runtime нет.

Предыдущие 76 сценариев проверяли клики и попытки размещения, но не завершение
строительства. Новый smoke подтверждает покупку, готовность (bit $2000),
потребление готового объекта, изменение конкретной ячейки карты и 600 кадров
после установки. Полные циклы бетонной плиты и ветрогенератора прошли с настоящим
YM2612/PSG. При установке реально наблюдался $02CF=$00, позже возврат к RET.
Для ветрогенератора тест сначала перемещает мешающего юнита обычной мышиной командой.

Мышь теперь обслуживает оба VBlank-обработчика карты, включая presenter $6092
с изменёнными границами курсора. Меню производства определяется по callback
$4504, page $13 и таблице $FFC63C. Видимая сетка: 3×6, x=32..127, y=48..191.
Наведение подаёт направление в исходный обработчик $847E, который сам меняет
подсветку и сведения; ЛКМ подаёт A в $28884 и входной byte $FFBF2B.
ПКМ/средняя кнопка наводит на EXIT и подтверждает его через тот же обработчик.
Очередь старого экрана сбрасывается при смене контекста.

Проверки: все 76 прежних комбинаций прошли с новым адаптером; fast core/SDL tests
покрывают новое меню и диапазон размещения; standalone Z80 live patch test прошёл.
Полное прохождение по-прежнему не проверено.

```sh
cc -std=c11 -O0 tools/test_dune_z80_patch.c -o build/test-dune-z80-patch -lm
./build/test-dune-z80-patch
cc -std=c11 -O0 -DGENESIS_AUDIO $(pkg-config --cflags sdl2) -c tools/smoke_dune_construction.c -o build/smoke-construction.o
c++ build/smoke-construction.o build/sound.o -o build/smoke-construction $(pkg-config --libs sdl2) -lm
./build/smoke-construction 0 on
./build/smoke-construction 1 on
```

Для двух последних команд нужен sound.o от обычной сборки ymfm. При компиляции
harness без GENESIS_AUDIO можно запускать те же сценарии с аргументом mute.


## Ранний бой за Харконненов: $044F36 / $044AE6

Пользователь сообщил остановку на $044F36 в начале боя. Это обработчик
команды юнита из ROM-таблицы $0FED7C, вызываемый оригинальным диспетчером
$171D0–$171E4. Загрузчики $16C12/$16C30 устанавливают таблицы обработчиков
сценариев: 64 записи по $0FED7C и 15 по $0FED40. Сборка теперь проверяет
адреса и переводит все записи этих таблиц, а не только встреченные при старте.
$044F36 также записан как подтверждённая точка входа в профиле.

В воспроизведённом раннем бою после гибели врага обнаружилась дополнительная
остановка на $044AE6 — недостающая ветка внутри уже переведённой функции.
Она добавлена в профиль. Проверка не подменяет опкоды и не меняет ROM.

Новый tools/smoke_dune_combat.c выбирает Харконненов исходными кнопками,
заходит в первую миссию, выбирает своего юнита и отдаёт команду атаки через
SDL mouse adapter. Проверяет оба обработанных клика, нулевое здоровье и
снятые флаги погибшего врага, затем продолжение симуляции. Координаты
пересчитываются по живому положению юнитов и камеры. В отличие от старого
boot smoke, Start прекращается при входе в миссию; учитываются оба штатных
обработчика карты $6092/$6D10.

```sh
cc -std=c11 -O1 -DGENESIS_AUDIO $(pkg-config --cflags sdl2) -c tools/smoke_dune_combat.c -o build/smoke-combat.o
c++ build/smoke-combat.o build/sound.o -o build/smoke-combat $(pkg-config --libs sdl2) -lm
./build/smoke-combat on
```

Холодный запуск combat smoke с настоящим YM2612/PSG прошёл до 8158 кадров:
оба клика обработаны, здоровье цели 37 → 0, флаги цели обнулены, fault отсутствует.
Регрессия всех 76 комбинаций мыши на новом переводе прошла без fault.
В воспроизведённом бою вызов именно $044F36 не наблюдался; его покрытие
подтверждено статической таблицей, а связь пользовательской остановки
конкретно со смертью юнита пока не установлена. Полное прохождение не проверено.


## Создание ракет: $0483C6

Остановка пользователя на $0483C6 произошла при открытом размещении постройки.
Адрес относится к созданию снарядов: функция $48352 выбирает тип через
`EXT.L D0; SUBI.L #18,D0; BMI; CMPI.L #6,D0; BGT` и signed word table
по $483B8. Типы 18–22 (Death Hand, Rocket, ARocket, GRocket, MiniRocket)
направляются на $483C6, типы 23–24 (Bullet, Sonic Blast) — на $48498.
Имена подтверждены исходными описателями типов по ROM-таблице $6C5BC.
Это может происходить одновременно со строительством: $44AFE вызывает
$48352 в боевой ветке.

Поиск signed switches теперь принимает SUBI.L вместе с EXT.L/SUBQ.L,
с прежней проверкой обеих границ и совпадения адресов таблицы у MOVE/JMP.
Переводится полный диапазон, а не только сообщённый адрес. В ROM ничего
не меняется; опкоды CPU по-прежнему не интерпретируются во время исполнения.

`tools/test_dune_projectile_switch.c` выполняет исходный диспетчер для всех
семи типов и значений вне границ (17, 25, -1), проверяет целевые PC и выполнение
первой инструкции каждого обработчика. Все 10 проверок прошли.
Дополнительно тест с холодного старта загружает первую миссию и на отдельных
копиях её состояния вызывает оригинальную функцию $48352 через исходный ABI.
Для каждого из семи типов проверяет полный возврат функции и создание
игрового объекта соответствующего типа. Это тест нативной функции, а не
воспроизведение ракетного выстрела только действиями мыши.

Все семь полных нативных вызовов создания снарядов прошли: 1743–3189
переведённых инструкций на вызов, корректный объект каждого типа, без fault.
В игровом mouse smoke ракетный выстрел не наблюдался; атака и гибель врага
прошли без fault, а целевой обработчик покрыт отдельной нативной проверкой.

```sh
cc -std=c11 -O1 tools/test_dune_projectile_switch.c -o build/test-projectiles -lm
./build/test-projectiles
```

На свежем переводе повторён полный mouse construction cycle с Windtrap и
настоящим YM2612/PSG: покупка, готовность, установка, 600 кадров после неё,
повторный вход в производство и выход правой кнопкой. До 6708 кадров, без fault.


## Автоматическое демо без ввода: $041978

Пользователь сообщил чёрный экран с остановкой на $041978 ещё до начала игры,
при ожидании без нажатий. Адрес — обработчик экранного сценария в
демонстрации. Оригинальный диспетчер $418DC использует signed word offsets
по $41936 и косвенный JSR по $41908; нулевой opcode завершает сценарий
до этого вызова. В профиле теперь указана вся таблица: 26 ненулевых
обработчиков, включая $41978. Build tool проверяет адреса и переводит их
вместе с остальными точками входа, сохраняя оригинальный сценарий демо.

Раньше boot smoke нажимал Start и обходил этот путь. Новый
`tools/smoke_dune_intro.c` не подаёт никаких кнопок или событий мыши.
Холодный запуск с настоящим YM2612/PSG прошёл 14400 кадров (4 минуты NTSC),
$41978 выполнился дважды, fault отсутствует. Финальный кадр — демонстрация
Refinery/переработки спайса, изображение визуально проверено, не чёрное.

```sh
cc -std=c11 -O1 -DGENESIS_AUDIO -c tools/smoke_dune_intro.c -o build/smoke-intro.o
c++ build/smoke-intro.o build/sound.o -o build/smoke-intro -lm
./build/smoke-intro 14400 on
```


## Поиск объекта при размещении: $02E36C

Пользователь сообщил остановку $02E36C на экране размещения постройки.
Это ветка поиска юнита в helper $2E35A: две старшие tag bits ссылки
маскируются через ANDI.W #$C000 и ROL.W #3 превращает их в смещения
0, 2, 4, 6 для JMP в таблицу исполняемых инструкций по $2E36A.
Юнит идёт через $2E36C → $2E384, постройка — через $2E36E → $2E374;
для пустой ссылки и клетки карты возвращается нулевой указатель.

Build tool автоматически распознаёт эту ограниченную комбинацию маски,
вращения и PC-indexed JMP, проверяет и переводит все четыре точки входа.
Так же покрыты соседние helpers получения координат ($2E25E) и проверки
валидности ($2E302), использующие тот же тип диспетчеризации. Добавлено
18 инструкций, без изменения ROM и без runtime CPU interpreter.

`tools/test_dune_references.c` загружает миссию с холодного старта и на её
копиях проверяет все четыре типа ссылки через исходный ABI трёх helpers.
Подтверждает указатели живого юнита и базы, корректность ссылок и координаты
юнита/клетки. Все 12 нативных вызовов завершились без fault, включая $2E36C.

```sh
cc -std=c11 -O1 tools/test_dune_references.c -o build/test-references -lm
./build/test-references
```

На свежем переводе повторён mouse construction cycle с Windtrap и настоящим
YM2612/PSG: покупка, готовность, установка, 600 кадров после неё, повторный
вход в производство и выход правой кнопкой. До 6708 кадров, без fault.
Большой footprint постройки на пользовательском скриншоте отдельно
не воспроизведён; пропущенный поиск юнита покрыт прямым нативным тестом.


## Переработка спайса: $00D06C

Пользователь сообщил $00D06C при построенном Refinery и прибывшем транспорте.
Это оригинальный обработчик переработки груза харвестера: через $FFD5AC
получает здание, через его child index — харвестер, уменьшает груз по +94
и начисляет кредиты в структуру дома. Проверка ненулевого груза ведёт к
$D1B6, где выполняется SUB.B D3,94(A3).

Обработчик входит в отдельную таблицу команд построек $6B9A0 (25 записей),
устанавливаемую загрузчиком $16C4E. Весь массив добавлен в callback_tables
профиля, и существующий build tool проверяет и переводит все его записи.
Добавлено 1338 инструкций. Runtime CPU interpreter не используется.

`tools/smoke_dune_refinery.c` выполняет холодный старт с настоящим YM2612/PSG,
мышью строит Windtrap, затем покупает и устанавливает Refinery ниже базы,
ждёт доставки харвестера и работы завода 12000 дополнительных кадров.
Проверяет наличие харвестера своего дома, вызовы $D06C и выполнение
выгрузки груза по $D1B6. Без подмены игровых объектов или ROM.

Проверка прошла до 19560 кадров (326 секунд NTSC): 103 вызова $D06C,
101 шаг выгрузки спайса, один живой харвестер своего дома, fault отсутствует.

```sh
cc -std=c11 -O1 -DGENESIS_AUDIO $(pkg-config --cflags sdl2) -c tools/smoke_dune_refinery.c -o build/smoke-refinery.o
c++ build/smoke-refinery.o build/sound.o -o build/smoke-refinery $(pkg-config --libs sdl2) -lm
./build/smoke-refinery 1 on
```


## Завершение миссии: $00B544

Пользователь сообщил остановку $00B544 при 1005 кредитах. Это экранный
VBlank callback анимации перехода после выполнения цели миссии. Сцена
по $26A2C передаёт его постоянный адрес через стек в $4796; эта функция
устанавливает callback в $FFE002. Раньше адрес не попадал в перевод.

Build tool теперь находит прямую передачу постоянного ROM-адреса через
MOVE.L #callback,(A7) перед JSR $4796, проверяет его и добавляет как точку
входа при каждом проходе discovery. Добавлено 9 инструкций $B544–$B572;
оригинальная анимация и игровой сценарий сохранены.

Прежний refinery smoke заканчивался до цели по кредитам. Теперь после
стройки завода и разгрузки харвестера продолжает добычу без подмены
кредитов/флагов, требует вход в $B544 и ещё 1800 кадров после него.
Холодный запуск с настоящим YM2612/PSG прошёл до 28860 кадров (481 секунд),
callback выполнился 207 раз, fault отсутствует. Финальный экран Victory!
визуально проверен в build/mission-completion.ppm.

Для воспроизведения используется прежняя команда:

```sh
./build/smoke-refinery 1 on
```

Проверено завершение первой миссии за Атрейдесов; дальнейший переход к
следующей миссии в этом прогоне не выполнялся.

## Мышь во всех меню построек

Контекст меню теперь определяется по выбранной постройке и её native screen
callback $4504, а не только по таблице Construction Yard. Поддерживаются
производственные меню заводов/казарм, ремонт/продажа остальных построек,
действие дворца и отдельная сетка космопорта ($BFC8/$BFCC).
IX и стены не открывают меню команд в оригинале.
Наведение проходит через доступные клетки штатными стрелками; поиск пути
обходит пустые клетки, встречающиеся у заводов. ЛКМ подаёт штатную A,
ПКМ/средняя кнопка выбирают EXIT. Стоимость, доступность, покупка и команды
остаются в оригинальной логике ROM.

Adapter/SDL tests проверяют контексты, обе сетки, выход и обход пустой клетки.
`tools/test_dune_building_menus.c` использует холодный запуск первой миссии,
а затем отдельные fixtures с заменой типа выбранной постройки: оригинальные
меню открываются настоящими SDL кликами, подсветка проверяется для каждого
доступного пункта, выход выполняется ПКМ. Это проверка меню всех типов,
а не прохождение поздних миссий или постройка всех зданий в кампании.
Космопорт использует собственную таблицу веток подсветки $9226/$9228/$922A/$922C;
в профиль добавлены все четыре точки входа. Они раскрывают ещё 25 инструкций,
итого 40348 M68K инструкций. Выход проверяется и ЛКМ по EXIT, и ПКМ.

## Мышь: главное меню и заставки

Наведение в главном меню переключает штатную подсветку START GAME / OPTIONS /
TUTORIAL; клик подтверждает пункт. На экране выбора дома наведение запускает
оригинальную анимацию перехода между гербами, клик ждёт её окончания и выбирает
дом. В YES/NO диалоге положение мыши выбирает ответ. На вступлении, сюжетных
экранах и инструкциях клик подаёт разовое оригинальное A для продолжения/пропуска.

Экраны определяются по native продолжениям чтения pad ($4724, $17D22,
$17E96/$17EA6/$17EB6, $4D4E, $4938, $25CB2); главный экран отличает return
address $178D2. Сохраняется приоритет клавиатуры. Клик не удерживается, очередь
на заставках ограничена одним действием, после подтверждения действует пауза
20 кадров; смена обработчика, его вызывающего адреса или экранного callback удаляет старое действие.
Ранние фазы загрузки, где ROM не читает pad, остаются со штатной длительностью.

`tools/test_dune_front_mouse.c` запускает игру с нуля, пропускает вступление
только SDL кликом (главное меню на кадре 991), проверяет наведение и оригинальные
результаты всех трёх пунктов. Затем отдельными прогонами доходит до первой
миссии за Атрейдесов, Ордосов и Харконненов только мышью, включая подтверждение
дома и сюжетные экраны. Также проверяется открытие OPTIONS без fault и дальнейшее открытие меню
Construction Yard мышью после вступления за каждый дом.
Это проверка главного меню; отдельные регулировки внутри OPTIONS здесь
не проверены и не получили координатного управления.

```sh
cc -std=c11 -O1 $(pkg-config --cflags sdl2) tools/test_dune_front_mouse.c -o build/test-front-mouse $(pkg-config --libs sdl2) -lm
./build/test-front-mouse
```

## Расширенный обзор и адаптация окна

Колесо на карте меняет масштаб 50–100% с шагом 10%; `0` возвращает 100%,
F11 переключает desktop fullscreen. Обзор подстраивается под размер окна
без растягивания изображения. На широком окне видна дополнительная местность;
портреты и кредиты закреплены справа сверху, миникарта справа снизу.
Их масштаб не зависит от зума карты. Меню и заставки сохраняют исходные
пропорции и координатное управление мышью. Начальный размер окна учитывает
доступное место на рабочем столе.

Это расширение видимой сцены: карта восстанавливается из оригинальных
метатайлов ROM $4ADF0 и текущих записей RAM $FF7D9C. Туман войны сохранён.
Спрайты строит оригинальная статически переведённая процедура $1088 на
одноразовой копии CPU, без продвижения игрового времени. Проверка сравнивает
CPU до и после рендера побайтно. В исходном окне 960×672 при 100% результат
совпадает с исходным кадром пиксель в пиксель. Максимальная сцена 1024×768:
на экстремально широких/высоких окнах масштаб ограничивается этим размером.

Клики на расширенной карте хранят абсолютные мировые координаты. Если цель
вне старого экрана, оригинальная процедура прокрутки $78D0 сначала подводит
камеру, затем штатный курсор и A/B выполняют действие. Цели вне доступной области
миссии отбрасываются. Выбор, приказы и установка зданий
остаются в оригинальной логике ROM.

`tools/test_dune_view.c` запускает первую миссию с нуля и проверяет реальный
SDL software renderer на 30 сочетаниях размеров окна и масштаба, включая
вертикальное окно и соотношение 4:1. Проверены первое рисование после resize,
координаты HUD, выбор юнита вне прежнего обзора, приказ с последующим движением,
меню Construction Yard и установка плиты через масштабированные SDL клики,
колесо и сброс масштаба. Прогон прошёл без fault. Desktop fullscreen и
ускоренный GPU renderer требуют проверки на обычном рабочем столе.

```sh
cc -std=c11 -O1 $(pkg-config --cflags sdl2) tools/test_dune_view.c -o build/test-dune-view $(pkg-config --libs sdl2) -lm
./build/test-dune-view
```

### Исправления по записи экрана от 6 октября

В первой версии расширенного обзора за курсор ошибочно принимался указатель
$BF60 (мировой маркер приказа), а настоящий указатель $BF18 оставался в HUD.
Из-за этого курсор рисовался дважды, со смещением и частями. Теперь $BF18
рисуется в мировом слое; $BF60 сохраняет исходное поведение. Учтены разные
правила выбора клетки: grid callback $6092 прибавляет 16 к координатам,
free callback $6D10 использует их прямо. Сетка курсора привязывается к клеткам,
свободный курсор следует указателю мыши.

Оригинальная камера следит за удалением pad-курсора от центра. Это вызывало
непрерывный уход карты даже при простом наведении внутри окна. При работе
мышью такое следование отключено. Мировой обзор сохраняет отдельную позицию;
внутренняя прокрутка для наведения/клика за пределами исходного экрана не
сдвигает обзор пользователя. У границ окна действует явная прокрутка мышью,
она останавливается при возвращении указателя внутрь. Колесо сохраняет
мировую точку под указателем, а не сдвигает карту при каждом изменении зума.

Новые проверки проходят с холодного запуска за Атрейдесов и Харконненов:
форма курсора на стыках рендера, отсутствие его в HUD, отсутствие прокрутки
при наведении внутри окна, неподвижность обзора при выборе дальнего юнита,
совпадение клетки предпросмотра с целью установки. Проверены якорь зума и
начало/окончание прокрутки на границе окна. Набор по-прежнему проверяет
30 раскладок, выбор/движение юнита, меню строительства и установку плиты.

```sh
./build/test-dune-view --hark
```

### Наведение на миникарту

Одна alpha-маска не отделяла миникарту от игрового курсора: RGB брался из
готового кадра, где курсор или красный/зелёный предпросмотр уже перекрывал
непрозрачный прямоугольник миникарты. Теперь RGB интерфейса строится
оригинальным VDP renderer на копии CPU со спрайтами только интерфейса.
При наведении на HUD мировой курсор скрывается; копирование центральной
части готового кадра в этом состоянии отключено, чтобы не вернуть его копию.

Regression fixture рисует настоящий native cursor поверх миникарты в
исходном кадре, проверяет наличие перекрытия, затем требует неизменности
чистого интерфейса и мирового слоя. Проверка повторяется с готовой постройкой.
Настоящее SDL наведение на миникарту во время установки скрывает предпросмотр;
возврат на карту и установка плиты проходят прежнюю проверку координат.

### Приказы мышью у границ миссии

Штатный pad-курсор ограничен отступом 24 px. На краю миссии камера уже не
может подвести его к последнему ряду, из-за чего клики мышью отбрасывались.
Адаптер оставляет pad-курсор в допустимых пределах, но после оригинального
расчёта клетки ($6486 / $701C) передаёт в $C240 точную мировую клетку мыши.
Выбор, движение и строительство по-прежнему выполняются кодом ROM.
Исправлен порядок границ камеры: Y — $E3FC/$E3FE, X — $E400/$E402.
Границы камеры учитывают размер исходного экрана при проверке площади миссии.

Быстрые проверки покрывают четыре угла в обоих режимах курсора (grid/free),
сохранение клика до оригинального A/B и точность итоговой клетки.

Native SDL-прогон `./build/test-dune-view --hark-border` с холодного запуска
за Харконненов отправляет существующий quad в первую строку миссии: Y=528
при минимуме Y=512, затем снимает выбор и снова выбирает юнит мышью.
Каждый игровой кадр обновляет SDL view и наведение, как обычный runtime.
Отдельный сценарий производства харвестера пока не подтверждён; проверка
приказа у границы выполнена на quad, адаптер координат общий для всех юнитов.

### Перенос обзора по миникарте

ЛКМ по миникарте центрирует независимый обзор на выбранной мировой клетке
(один пиксель миникарты — клетка 32 px), с ограничением по границам камеры
миссии. Масштаб сохраняется, A/B в ROM не отправляется, выбранный юнит
сохраняется. Используется существующее преобразование координат HUD,
включая размер окна и HiDPI. По просьбе пользователя изменение не собиралось
и не тестировалось; требуется ручная проверка.

## Локальный macOS app bundle — 2026-10-07

Добавлен `tools/build_macos_app.py`: упаковка готовой SDL-сборки в .app
без окна Терминала, с локальной подписью и журналом в Application Support.
Используются установленные SDL-библиотеки. Синтетическая проверка покрывает
структуру пакета, подпись, пробелы в пути и защиту от перезаписи приложения.

## Проверка macOS — 2026-10-07

На Apple Silicon с macOS 27.0, Python 3.14.8, Apple Clang и SDL2 2.32.74
исходный движок прошёл 290 тестов; пропущена одна проверка Linux `/dev/full`.
Синтетическое демо собрано в Mach-O arm64 и выдало ожидаемый результат RAM.
Проверены рабочая папка и передача аргументов лаунчером `run-dune.command`.
Исправленный поиск ресурсов через `_NSGetExecutablePath` и `realpath`
уже присутствует в исходном ReArrakis и прошёл проверки переноса и symlink.

Пользовательский ROM USA прошёл проверку SHA-256 и собрался штатным
сборщиком в arm64. На нём прошли mouse-only переходы от заставки до первой
миссии за все три дома, меню стройдвора, Z80 RET/NOP, создание снарядов и
encoded references. SDL software renderer прошёл 30 сочетаний окна/зума,
попиксельное сравнение исходного кадра, изоляцию курсора/HUD, приказы юнитам
и размещение зданий при увеличенной карте.

Также прошли штатные игровые проверки покупки/размещения ветряка,
боя Харконненов до уничтожения противника и добычи Атрейдесов до перехода
завершения миссии (27 360 кадров), без правок кредитов или флага победы.
Использованы `tools/smoke_dune_construction.c` (аргумент `1`),
`tools/smoke_dune_combat.c` и `tools/smoke_dune_refinery.c`, звук mute.

Headless-прогон 20 млн инструкций с ymfm: 1 347 522 стереокадра,
229 726 записей YM, без fault и потерь сэмплов. Выход по лимиту (код 2)
ожидаем. GPU, полный экран и физический аудиовыход требуют ручной проверки
на рабочем столе; эти результаты её не подменяют.

## Пароли и индикатор 68000 — 2026-10-07

Исправлен пропуск `$21A0C` при PLAYTESTER: профиль добавляет все 11 ветвей
таблицы действий паролей `$2197C`. Теперь сборка содержит 40 382 инструкции
M68K и 3 095 Z80. Нативная проверка ввела все 29 паролей через оригинальную
экранную клавиатуру кнопками контроллера и проверила эффекты обработчиков.
Дополнительно проверены повторное выключение LOOKAROUND/PLAYTESTER и три
неверных ввода. Экран пароля вызывается из снимка первой миссии; прохождение
всех открываемых миссий этим не проверялось.

F3 включает приблизительную занятость виртуального 68000 за 15 кадров.
Из общего числа циклов исключаются ожидание VBlank `$0FDA`/`$0FDE` и halt;
прерывания и другие неизвестные циклы ожидания считаются работой.
Это не загрузка процессора компьютера. Частота консоли и скорость игры
сохраняются. Прошли 17 проверок адаптеров/таймингов и оптимизированная
Linux-сборка. Сообщение X11 Compose — отдельная проблема, здесь не исправлена.

## Скорость консоли в ветке debug — 2026-10-07

По просьбе пользователя изменение изолировано в ветке `debug`. Панель F3
показывает SPD: скорость виртуального времени относительно реального,
с интервалом от 0,5 секунды. 100% не исключает торможения оригинальной
игровой логики. Пауза, остановка и откат часов сбрасывают измерение.

Эксперимент в debug: запуск с двойной частотой 68000, F4 переключает 1x/2x.
F3 показывает множитель (68K: 1/2). На каждый цикл CPU приходится вдвое
меньше времени периферии; VDP, Z80 и звук сохраняют штатную частоту.
Это может менять игровую логику, зависящую от CPU. SPD показывает скорость
консоли относительно реального времени, а не множитель CPU.

Мышь добавлена в оригинальные настройки и экран пароля: наведение выбирает
строку/клавишу, левый клик активирует. Для значений левая половина строки
переключает назад, правая — вперёд. В клавиатуре доступны буквы, `<`/`>`
и `!` с оригинальным действием. Правый клик закрывает настройки/пароль;
в подтверждении левый клик отвечает «да», правый — «нет». Изменения
выполняют оригинальные обработчики, без прямой записи значений настроек.
## Прокрутка от края игрового курсора — 2026-10-07

Порог прокрутки учитывает границы игрового квадрата: свободный курсор
(-13..12 относительно центра) и клетку 32×32 с привязкой к сетке.
Используется масштаб карты в drawable pixels, а не полоса 12 оконных
пикселей от системного указателя. SDL-регрессия проверяет четыре края,
масштабы 50/75/100%, остановку внутри окна и при выходе из него.

## 2026-10-07 — SDL2 gamepad and settings

Local branch `feat/dune-gamepad-settings` starts from debug `c7a49c5`, including
the 2x CPU experiment. Reused RROP's SDL controller input and bitmap font;
added a small F10/Back menu for layouts and A/B/C/Start assignment. No new
dependency. Configuration is `gamepad.cfg` in the working directory (the app
launcher uses Application Support/ReArrakis). Input is cleared across menu,
focus and disconnect transitions; direct pad input cancels mouse intents.

The initial diagnostic omitted SDL_INIT_VIDEO and reported no devices.
With video initialized, the user's normal launch recorded `usb gamepad`,
GUID `0500493cac05000004000000cda56d04`, recognized as an SDL controller,
with 13 buttons and real down/up events. This confirms SDL delivery, not yet
correct physical labels or full in-game control. A virtual SDL-controller
regression check exercises the frontend, remapping and persistence without ROM.

Validation: 292 tests passed (one Linux-only /dev/full check skipped).
The virtual-controller check passed again after the settings layout changed.
Native ROM build and ad-hoc app signature verification passed; a headless
2,000,000-instruction smoke run reached the expected budget without a fault.
The settings menu includes an SDL-drawn three-button Sega pad, highlighting
the current assignment. A dummy-renderer screenshot was visually checked.
Physical gameplay validation remains pending.

The settings illustration now traces the shell and button positions from
[Evan-Amos's public-domain controller photograph](https://commons.wikimedia.org/wiki/File:Sega-Genesis-3But-Cont.jpg).
The original aspect ratio and concave grip cutout are retained; menu choices
sit beside the pad. The photo is a reference only, with no image runtime dependency.

## 2026-10-07 — controller selection and analog cursor

F10 now cycles all recognized pads without unplugging them and remembers the
selected SDL GUID. If that pad disappears, another connected pad takes over.
Identical models share a GUID, so restart selects the first matching instance;
button mappings remain shared. Config versions 1/2 are accepted; version 3
stores controller preference and Cursor Left/Right/Off (Left by default).
Analog cursor movement reuses mouse intents, with deadzone, proportional speed,
window bounds and focus/menu guards. DUNE_STICK_SPEED provides 25–300% sensitivity.
macOS enumerated Xbox Wireless Controller over Bluetooth (045e:02fd); physical
in-game validation is pending. Virtual SDL tests exercise two simultaneous
controllers, selection/persistence, unplug fallback and both cursor sticks.

The final controller drawing uses a consistent front view, checked against
[this original three-button pad photograph](https://www.gamerlifestore.com/products/sega-genesis-3-button-controller-original).
The shell, circular D-pad recess, angled action-button recess and Start are
traced together. Curves render at drawable resolution; no photo is bundled.

Validation also cycles 21 simultaneously attached virtual pads and verifies
wraparound. The mouse-event fixture initially failed because its minimal host
excluded the new analog fields; analog servicing now lives in the frontend,
leaving the existing mouse adapter independent. Both adapter and controller
checks pass. The desktop pointer hides during stick use and returns on mouse
movement.

Final validation: 292 tests passed, one Linux-only check skipped. The updated
21-pad/controller test and four Dune adapter checks passed separately after
the final cursor-visibility change. Native build and app signature verified.

## 2026-10-07 — recoverable configuration and exclusive controller input

User reported an uncontrolled upper-left cursor and could not find settings.
Two code-level issues were corrected: disconnect previously selected another
available device, and remapping events from inactive controllers cleared active
input. Selection now pins one instance; disconnect requires a manual replacement,
even for matching GUIDs. Initial off-center analog state cannot drive the game
until the configured axes return to their calibrated deadzone. The exact cause
of the user's physical drift remains unverified without live axis readings.

Esc/F1/F10 open settings. Buttons / D-pad supports eight individual assignments,
an all-controls wizard, clear, duplicate swap, signed axes and triggers. Axes
settings expose X/Y choice, inversion, deadzone, sensitivity, center calibration
and RAW/CENTER telemetry. Analog menu navigation is disabled so drift cannot
change settings. Mouse gameplay can be disabled while settings keep mouse input.
Config version 4 adds per-GUID profiles, with versions 1–3 accepted and preserved
for initial migration. Identical GUIDs still share a profile; runtime selection
uses instance IDs. Shared cursor/digital axes are mutually exclusive.

Implementation references: [SDL axis ranges and deadzones](https://wiki.libsdl.org/SDL2/SDL_GameControllerAxis),
[SDL instance IDs](https://wiki.libsdl.org/SDL2/SDL_JoystickGetDeviceInstanceID),
and [RetroArch controller/remap separation](https://docs.libretro.com/guides/input-and-controls/).
These inform the implementation; RetroArch is not a dependency.

Validation: the complete 292-test run passed with one Linux-only /dev/full
skip. A final capture regression also passed: a pre-held unrelated button or
axis no longer prevents binding a fresh control; that held control itself must
be released before it can be assigned. Both ReArrakis.app and
ReArrakis-Gamepad.app are rebuilt locally, with previous versions kept in build/.
Native compilation and ad-hoc signature checks passed. Live physical drift
verification remains pending.


## 2026-10-08: pause menu and save states

- Esc/F10 now opens a game menu; F1 retains direct controller-settings access.
- Added ten local state slots, occupied/empty status and modification times;
  load/overwrite/quit have Cancel-focused confirmation. Pause and input handling
  remain shared with controller settings. Fullscreen, volume and CPU speed are
  exposed without inventing a second settings backend.
- Snapshot CPU/devices, scrub host pointers, serialize FM through the existing
  ymfm API, atomically replace files and validate before committing a load.
  Reset host audio/input/camera clocks after loading. Deliberate limit: matching
  ROM/runtime/features/native ABI only; no version migration or WAV-recording
  snapshots. This is separate from the game's original save mechanism.
- Menu references: [RetroArch Quick Menu](https://docs.libretro.com/guides/quick-menu/)
  for pause/resume and grouped state/control actions;
  [Game Accessibility Guidelines](https://gameaccessibilityguidelines.com/full-list/)
  for consistent input methods, readable contrast and visible focus. Confirmation
  and safe default selection are implementation choices for avoiding lost progress.
- Added SDL round-trip/corrupt-state/confirmation checks, an exact FM+PSG audio
  continuation test, and optional ROM-backed `tools/test_dune_state.c`.
- Validation: full suite 294 tests, one platform-specific skip; real-ROM frame
  5000 -> 5030 replay reproduced RAM, video, PC, master clock and FM state.
  Main/confirmation menus rendered and inspected with SDL's software renderer.
  Native app rebuilt; both ReArrakis.app and ReArrakis-Gamepad.app updated and
  their ad-hoc signatures verified. Physical controller navigation remains a
  user-side check; the automated menu/input checks use SDL events/virtual pads.

## 2026-10-08: pre-push audit of the gamepad/menu series

Reviewed the five commits from c7a49c5 through 7e5c26e, including input lifecycle,
profile parsing/persistence, menu actions, state loading, artwork bounds and the
publication file list. Found and corrected:

- P2: ignored physical mouse motion still reset the analog cursor owner. Gate
  that reset with the gameplay-mouse setting; virtual-pad regression verifies
  that disabled mouse input leaves the stick cursor and its coordinates intact.
- P2: C `rename` cannot replace existing files on Windows, affecting both
  profiles and save slots. Share a host-file replacement helper using
  MoveFileExA(REPLACE_EXISTING | WRITE_THROUGH) on Windows and rename elsewhere;
  never delete the previous file first. Added overwrite and failed-replacement
  coverage. References: [Microsoft CRT rename](https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/rename-wrename?view=msvc-170)
  and [MoveFileExA](https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-movefileexa).
- P2: state compatibility included the FM wrapper but omitted its vendored ymfm
  implementation. Hash the sorted backend headers/sources as well. A regression
  changes a backend source and verifies that the compatibility key changes.

ROM/extracted art/generated translations/binaries are absent from the series;
commit messages contain no agent attribution. Remaining limits: same-runtime/ABI
local snapshots, per-model controller profiles (identical models share settings),
and physical-controller navigation still needs user verification. Windows native
execution was not available on this Mac; the Windows API path was reviewed against
its platform documentation. Existing Mac/Linux replacement semantics are retained.

Validation after fixes: 295 tests, one platform-specific skip; ROM-backed
5000 -> 5030 state replay still matched RAM/video/PC/clocks/FM. Rebuilt both
local macOS apps and verified their ad-hoc signatures. The audit changes the
runtime fingerprint, so states from the pre-audit app require that older app.
