# LineFollower_Braia_Code
O algoritmo para esse seguidor de linha está separado em [serviços](./modules/robot/services/) que possuem cada um a sua função e executam paralelamente no microcontrolador, sendo os serviços utilizados atualmente pelo robô os listados abaixo:

- [CarStatusService](./modules/robot/services/CarStatusService/)
- [LEDsService](./modules/robot/services/LEDsService/)
- [MappingService](./modules/robot/services/MappingService/)
- [MotorsService](./modules/robot/services/MotorsService/)
- [PIDService](./modules/robot/services/PIDService/)
- [SensorsService](./modules/robot/services/SensorsService/)
- [SpeedService](./modules/robot/services/SpeedService/)

Além disso, os dados utilizados pelo robô são armazenados em classes, em que, cada classe tratará dos dados de sistemas ou funções específicas, sendo essas classes as listadas abaixo:

- [dataPID](./modules/shared/components/RobotData/src/PID/)
- [dataSensor](./modules/shared/components/RobotData/src/Sensor/)
- [dataMapping](./modules/shared/components/RobotData/src/Mapping/)
- [dataSpeed](./modules/shared/components/RobotData/src/Speed/)
- [RobotStatus](./modules/shared/components/RobotData/src/Status/)

Todas essas classes de dados relacionadas ao robô estão localizadas na pasta [RobotData](./modules/shared/components/RobotData/).

Além disso, com o intuito de possibilitar a comunicação à distância com o robô e até mesmo a implementação da nossa dashboard para enviar ou receber comandos, desenvolvemos um gateway que trata os comandos que devem ser enviados ao robô, hospeda a dashboard que criamos e age como um meio de comunicação entre o usuário e o carrinho, evitando que essas tarefas sobrecarreguem o microcontrolador do seguidor de linha, o código desse gateway também funciona a base de [serviços](./modules/gateway/services/), sendo eles:

- [SerialService](./modules/gateway/services/SerialService/)
- [ServerService](./modules/gateway/services/ServerService/)

A comunicação entre o robô e o gateway é feita utilizando o protocolo espnow e a comunicação do gateway com o dashboard utiliza websocket.

Para mais informações sobre o dashboard, acesse o repositório a seguir: [LineFollower_CCenter_Code](https://github.com/Tamandutech/LineFollower_CCenter_Code) 
