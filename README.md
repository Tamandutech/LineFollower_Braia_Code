# 🤖 Seguidor de Linha - Braia

![Versão](https://img.shields.io/badge/versão-0.0-blue)
![Linguagem](https://img.shields.io/badge/STM32-C/C++-brightgreen)

## Alterações Necessárias Celeris Core v1

- Circuito de proteção de corrente inversa com interruptor não funcionou, usar somente mosfet com interruptor e manter conectores xt para não ocorrer ligações invertidas

- Corrigir leds no esquemáticos que ficaram com a pegada invertida

- Utilizar pino com pwm comum para leds endereçaveis (atualmente em pwm invertido)

- utilizar leds maiores (0805 exemplo) não tem necessidade de utilizar leds tão pequenos, apenas dificultam manutenção

- remover leds de pinos de programação, não tem necessidade de utilizar e podem eceder o limite de corrente do pino

- corrigir pull up/down no pino de boot do stm32


## 🔧 Requisitos

### Obrigatórios
- STM32CubeIDE 1.18.0 

### Opcionais
Para desenvolvimento através do Visual Studio Code: 
- STM32CubeCLT 1.18.0 (necessário para fazer upload do firmware e depuração) 
- Pacote de extensão C/C++ para Visual Studio Code (necessário para intellisense) 
- Extensão Cortex-Debug para Visual Studio Code (necessário para depuração) 
- STM32CubeMX 6.14.0 (recomendado para editar .ioc sem o STM32CubeIDE aberto) 

## 🚀 Como Usar

### Configuração com STM32CubeIDE
1. Clone o repositório:
   - Para clonar **apenas a branch de desenvolvimento do STM32**, use:
     ```bash
     git clone --branch develop-stm32 --single-branch https://github.com/Tamandutech/LineFollower_Braia_Code.git
     ```
   - Para clonar o **repositório completo**, use:
     ```bash
     git clone --branch develop-stm32 https://github.com/Tamandutech/LineFollower_Braia_Code.git
     ```

2. Abra o STM32CubeIDE

3. Importe o projeto através de:
   ```
   File > Open Projects from File System
   ```

### Configuração com Visual Studio Code  
*(Temporariamente apenas para Windows)*


1. Siga os passos 1-3 da seção anterior "Configuração com STM32CubeIDE"  

2. Abra o Visual Studio Code  

3. **Compile e/ou faça o upload do firmware**  
   No terminal do Visual Studio Code, execute um dos comandos abaixo:
   - Para compilar o projeto:  
     ```bash
     .\STMCube_cli_helper.bat build
     ```
   - Para fazer o upload do firmware no dispositivo STM32:  
     ```bash
     .\STMCube_cli_helper.bat flash
     ```
   - Para compilar e fazer o upload do firmware em sequência:  
     ```bash
     .\STMCube_cli_helper.bat all
     ```

4. **Debug**  
   Para iniciar o debug utilize o atalho "F5" ou clique no botão "Iniciar Depuração" na barra lateral esquerda do Visual Studio Code. 

#### Observações
O arquivo de lote `STMCube_cli_helper.bat` é um script que automatiza o processo de compilação e upload do firmware no dispositivo STM32. Ele utiliza o STM32CubeCLT e a STM32CubeIDE para realizar essas operações. 

- Não é possível compilar o código quando o STM32CubeIDE estiver aberto, pois ele bloqueia o acesso ao workspace. 

- Não existem restrições para a função de upload do firmware, ou seja, o STM32CubeIDE pode estar aberto ou fechado. 

- Ao utilizar a função de debug pelo VSCode, por padrão o codigo não está sendo compilado para otimizar o tempo de execução. Então lembre sempre de compilar antes de Debugar, ou descomente a linha ` // "preLaunchTask": "Build",` no arquivo `.vscode\launch.json` para sempre compilar o codigo antes de iniciar a sessão de debug. 

- É recomendado realizar a instalação padrão do STM32CubeCLT e STM32CubeIDE (e utilizar o workspace padrão) 

Caso tenha instalado em locais diferentes, será necessário modificar:
- O arquivo `STMCube_cli_helper.bat` para apontar para os caminhos corretos.
- O arquivo `.vscode\c_cpp_properties.json` para ajustar as configurações.



