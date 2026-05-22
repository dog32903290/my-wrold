# 我的世界 — Native Canvas 開發指南

## 起手 Skills

討論或實作本 repo 時，先用：

- `native-canvas-spine` — 本 repo 是「柏為的畫布」native 化的新身體；第一階段鎖 shader preview、audio analyzer spine、compound patcher、AI worker commandGraph。
- `cpp-patch-runtime` — 設計 graph contract、runtimeGraph、headless/debug proof、C++ runtime 邊界時用。
- `low-latency-audio-spine` — 涉及 CoreAudio、MIDI、buffer size、realtime callback、device routing 時用。
- `analyzer-patch-architect` — 拆 loudness / attack / density / sustain / silence 等 analyzer patch 時用。

## 第一階段三根線

```text
V1 visual proof
native app -> hand-written Shader node -> Output preview

A1 audio proof
native preferences -> audio input -> native analyzer meter rows

C1 compound proof
loudness compound patcher -> expanded child patchers -> collapsed public ports
```

不要用漂亮材料 demo 取代這三根線。黑絨布、memory trail、舊作品 migration 都在三根線承重後再進來。

## 技術預設

- App framework：JUCE + CMake。
- Display name：`我的世界`。
- Internal target / package id：ASCII，暫定 `my-world`。
- Shader backend：第一版 OpenGL/GLSL 只做 proof；production GPU 方向鎖 Mac-first Metal。`RenderBackend` 邊界一開始就要留，MoltenVK/Vulkan 先不作為第一後端。
- First app type：standalone native app，不先做 plugin。
- Repo policy：小步 commit；每根 proof 都要有可回讀證據。

## 計劃同步規則

- 每次改 code 後，commit 前都要回頭檢查最新的 plan / spec 文件。
- 計劃不能只新增不刪減：已證明的要標成已證明，錯的假設要拆掉或改寫，停車問題要保留但不要混在當前承重線裡。
- commit 必須包含對應的 plan 更新，除非這次改動純屬格式化或修 typo，且不改變任何工程狀態。

## 結構規則

- C++ UI 要 data-driven。不要把 UI、runtime、serialization、ports 全寫進巨大 node class。
- 所有改 graph 的行為都走 command path。AI worker、UI、人手 script 不准各走一條。
- Graph 至少分清：`editorGraph`、`runtimeGraph`、`commandGraph`、`collaborationLog`。
- Compound patcher 是第一階段骨架，不是之後的整理功能。
- Audio analyzer 不准整包變黑盒節點；先拆 Raw Facts / Feature / Detector / Aggregate / Output Shaping。

## 第一個 shader proof 合約

```text
hand-written GLSL
-> compile
-> native preview
-> edit live
```

- Compile 成功：替換 live program。
- Compile 失敗：保留上一個成功畫面。
- 錯誤訊息要可見。
- Proof 可 dump `frame.png`、`cook_order.json`、`node_stats.json`。
