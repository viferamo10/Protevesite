# PROTEVE FW-S3-MVP-001 — Comissionamento da Sixspan ESP32-S3

Bem-vindo ao primeiro deploy do núcleo P0. Este guia leva a placa do zero ao primeiro boot em ~20 minutos, sem precisar saber programar.

## Regra de ouro do primeiro boot

**Não conecte NADA na placa além do cabo USB.** Nada de RS485, PZEM, relé, RTC ou SD. O primeiro boot é só pra provar que o coração bate. A bancada de sensores vem depois, comigo te guiando (e conferindo o nível lógico do RS485 — 5V pode queimar a UART do S3).

## Passo a passo

1. **Instale o VS Code** (gratuito): https://code.visualstudio.com
2. **Instale a extensão PlatformIO IDE**: abra o VS Code → ícone de blocos na barra lateral (Extensões) → busque "PlatformIO IDE" → Install. Depois da instalação, ele pede pra recarregar; espere aparecer o ícone da formiga na barra lateral esquerda.
3. **Abra este projeto**: File → Open Folder → selecione a pasta que você extraiu deste ZIP (a que tem o `platformio.ini`).
4. **Plugue a placa** com cabo USB-C **de dados** (aviso importante: muito cabo é só de carga — se não aparecer porta nenhuma, troque o cabo antes de culpar a placa). Use a porta USB-C de baixo da Sixspan.
5. **Compile**: clique no ✓ (PlatformIO: Build). A primeira vez demora — baavaa o toolchain inteiro (5-10 min). Nas seguintes, é rápido (13 segundos).
6. **Grave**: clique na seta → (Upload). Se travar em `Connecting.....`, segure o botão **BOOT** da placa até aparecer "escrevendo". É normal.
7. **Veja o coração bater**: abra o Monitor Serial (ícone de tomada na barra inferior) em 115200. Você vai ver o banner da PROTEVE e, a cada 30 segundos, o ciclo de telemetria.

## O que esperar no primeiro boot (comportamento NORMAL)

- Banner `PROTEVE FW-S3-MVP-001 v0.1.0-dev`
- Telemetria a cada 30s (ciclo com gate de RTC)
- PZEM-1 e PZEM-2 reportando **OFFLINE** (não há sensores ligados — é esperado)
- Estados de dado **SIMULATED/STALE** (o sistema é honesto sobre o que não vê)
- Zero atuação: o GPIO de atuação é **-1 (ALARM-ONLY)** — este firmware NÃO liga nem desliga nada por enquanto. Ele só observa, aprende e avisa. A atuação física só nasce depois da validação completa na bancada.

## Depois do primeiro boot

Me manda o print da saída do Monitor Serial que a gente parte pra bancada:
1. Rotação de credenciais (BotFather /revoke no token exposto)
2. RS485: conferir 5V vs 3.3V ANTES de ligar qualquer PZEM
3. Validação da pinagem canônica (a bancada manda)
4. Timeout do PZEM-3 (a pendência que veio do V4.1)

## Estrutura do projeto

```
platformio.ini   → configuração da placa (Sixspan N16R8, qio_opi, USB CDC)
src/              → o firmware (main.cpp + módulos do núcleo P0)
include/          → headers (Types, ProtectionEngine, NILM, etc.)
docs/             → documentação técnica
tests/            → testes unitários
```
