# Seguidor de linha - Braia
O algoritmo para esse seguidor de linha tem suas classes e arquivos separados em pastas de acordo com as suas funções e responsabilidades que podem ser as listadas abaixo:
 - Drivers (interface entre software e hardware)
 - Serviços (camada de aplicação)
 - Estrutura de dados
 - Gerenciamento e armazenamento de dados
 - Tratamento e execução de comandos do bluetooth

## Drivers
Atualmente, os drivers para cada componente de hardware do robô estão armazenados na pasta [robot/components](./modules/robot/components), sendo os drivers responsáveis por se comunicar ou receber informações dos hardwares instalados no robô, abaixo estão listadas as pastas que contém drivers e a sua função:
  - [ESP32Encoder](./modules/robot/components/ESP32Encoder): leitura dos sinais enviados pelos encoders
  - [ESP32MotorControl](./modules/robot/components/ESP32MotorControl): acionamento dos motores, através da ponte H
  - [LedStripEncoder](./modules/robot/components/LedStripEncoder): controle dos Leds RGB endereçaveis
  - [MCP3008](./modules/robot/components/MCP3008): comunicação com o ADC externo
  - [QTRSensors](./modules/robot/components/QTRSensors): leitura e calibração dos sensores infravermelhos

## Serviços
Os serviços do robô estão em sua maioria na pasta [robot/services](./modules/robot/services/), neles que a lógica utilizada no robô para executar as suas tarefas é implementada, eles possuem cada um a sua função e geralmente executam paralelamente no microcontrolador, atráves das tasks do FreeRTOS, ou auxiliam outros serviços nas suas funções, os serviços utilizados atualmente pelo robô e suas tarefas principais estão listadas abaixo:

- [CarStatusService](./modules/robot/services/CarStatusService/): responsável pela definição do estado do robô com base na pista e nos dados armazenados nele
- [LEDsService](./modules/robot/services/LEDsService/): responsável por receber comandos de outros serviços para o acionamento dos leds
- [MappingService](./modules/robot/services/MappingService/): responsável por criar o mapeamento quando o robô está no modo mapeamento
- [PIDService](./modules/robot/services/PIDService/): responsável pelas estratégias de controle implementadas no robô
- [SensorsService](./modules/robot/services/SensorsService/): responsável pela leitura e calibração dos sensores IR frontais e laterais
- [SpeedService](./modules/robot/services/SpeedService/): responsável pelo cálculo da velocidade e deslocamento do robô ou de suas rodas
- [BLEServerService](./modules/common/services/BLEServerService): responsável pela comunicação bluetooth do robô com a dashboard

## Gerenciamento e armazenamento de dados
Os parâmetros do robô e mapeamento são armazenados na memória flash permitindo que eles sejam salvos no robô, entretanto para possibilitar o gerenciamento desses parâmetros e facilitar o armazenamento e carregamento deles da memória flash, criamos a pasta [DataObjects](modules/common/components/DataObjects), onde se encontram as classes encarregadas de gerenciar os dados e parâmetros armazenados nas estruturas de dados do robô e também de lidar com o armazenamento do ESP32 para facilitar o uso da flash, as classes que estão nessas pastas são lisatadas abaixo:

- [DataAbstract](./modules/common/components/DataObjects/src/DataAbstract.hpp): classe responsável por lidar com os parâmetros do robô, permitindo a leitura e escrita deles em tempo de execução, funcionando como um tipo genérico para diferentes tipos de parâmetros. 

- [DataMap](./modules/common/components/DataObjects/src/DataMap.hpp): classe responsável por lidar com os dados do mapeamento.

- [DataStorage](./modules/common/components/DataObjects/src/DataStorage.hpp): classe responsável por carregar parâmetros da flash ou salvá-los nela, através de arquivos.

- [DataManager](./modules/common/components/DataObjects/src/DataManager.hpp): classe responsável por gerenciar os parâmetros do robô através de uma lista de parâmetros que o permite facilmente acessá-los quando necessário.

- [IDataAbstract](./modules/common/components/DataObjects/src/IDataAbstract.hpp): Interface utilizada como base para a classe DataMap e DataAbstract.

## Estrutura de dados
Os dados utilizados pelo robô são armazenados em classes, em que, cada classe tratará dos dados de sistemas ou funções específicas, sendo essas classes as listadas abaixo:

- [dataPID](./modules/robot/components/RobotData/src/PID/): dados e parãmetros sobre o controlador PID do robô
- [dataSensor](./modules/robot/components/RobotData/src/Sensor/): dados sobre as leituras dos sensores infravermelho
- [dataSLatMarks](./modules/robot/components/RobotData/src/SLatMarks/): dados e parâmetros sobre o mapeamento
- [dataSpeed](./modules/robot/components/RobotData/src/Speed/): dados e parâmetros sobre posição, velocidade e aceleração
- [RobotStatus](./modules/robot/components/RobotData/src/Status/): dados e parâmetros sobre o estado do robô

Todas essas classes de dados relacionadas ao robô estão localizadas na pasta [RobotData](./modules/robot/components/RobotData/) que também armazena algumas enums e maps (dicionários) do robô.

## Tratamento e execução de comandos do bluetooth

O robô também possui uma dashboard hospedada no firebase que permite a edição e backup de parâmetros, alteração e backup do mapeamento da pista e geração de gráficos em tempo real, e com o intuito de possibilitar a comunicação à distância da dash com o robô, utilizamos comunicação bluetooth (BLE GATT), sendo o serviço [BLEService](./modules/common/services/BLEServerService) responsável por tratar dessa comunicação sem fio com a dashboard, sendo ele responsável pela interpretação dos comandos enviados pela dash e retorno dos dados solicitados. Os comandos recebidos e enviados pelo robô são comandos de texto e para simplificar a interpretação deles utilizamos a biblioteca [BetterConsole](modules/common/components/BetterConsole) para extrair as partes importantes dos comandos e executar as funções relativas à esses comandos que estão na pasta [CMDWrapper](modules/common/components/CMDWrapper) que encapsula a função executada por cada comando.

Para mais informações sobre o dashboard, acesse o repositório a seguir: [LineFollower_CCenter_Code](https://github.com/Tamandutech/LineFollower_CCenter_Code) 
