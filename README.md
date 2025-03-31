# 🤖 Seguidor de Linha - Braia

![Versão](https://img.shields.io/badge/versão-0.0-blue)
![Linguagem](https://img.shields.io/badge/STM32-C/C++-brightgreen)

## 🔧 Requisitos

### Obrigatórios
- STM32CubeIDE 1.18.0 ou superior

### Opcionais
Para desenvolvimento através do Visual Studio Code:
- STM32CubeMX 6.14.0 ou superior
- STM32CubeCLT (mesma versão do STM32CubeIDE)

## 🚀 Como Usar

### Configuração com STM32CubeIDE
1. Clone o repositório:
   ```bash
   git clone [URL-do-repositório]
   ```
2. Abra o STM32CubeIDE
3. Importe o projeto através de:
   ```
   File > Open Projects from File System
   ```

### Configuração com Visual Studio Code  
(Temporariamente apenas para Windows)
1. Siga os passos 1-3 acima para configurar o projeto no STM32CubeIDE
2. Abra o arquivo `STMCube_cli_helper.bat` 
3. Modifique o caminho do workspace para o da sua CubeIDE
4. Se necessário, altere o nome do projeto no mesmo arquivo
5. Execute `STMCube_cli_helper.bat` para compilar ou fazer flash no dispositivo