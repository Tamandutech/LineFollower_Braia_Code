# PIDService
Essa pasta contém o serviço responsável pelo controle do robô, nele utilizamos controle PID para corrigir a posição do robô na linha e o controle de velocidade é feito em malha aberta assim como o controle da aceleração, ou seja, para a velocidade apenas é definido um duty cycle para o sinal PWM. As velocidades e acelerações do robô podem ser definidas na dashboard assim como as constantes do PID.

Esse serviço, funciona da seguinte forma, primeiramente em seu construtor o driver dos motores e a estrutura do PID é inicializada, após isso, um timer é configurado para garantir que o serviço execute periodicamente em um tempo fixo definido no código. Após a finalização do construtor, o serviço estará pronto para começar a rodar em loop, onde executará a cada loop as etapas abaixo:

1. Carrega dados sobre a posição do robô na pista e o estado do robô com o intuito de escolher velocidades, acelerações e constantes PID adequadas para o robô em cada situação.
2. Define a aceleração, desaceleração, aceleração inicial e velocidade inicia do robô.
3. Carrega as constantes do PID com base no segmento da pista que o robô está e/ou seu estado.
4. Calcula a velocudade rotacional e linear do robô para análise em gráficos na dashboard.
5. Calcula o erro de posição do robô em relação à linha e armazena dados relacionados à esses erros, sendo esses dados possíveis de serem analisados por gráficos na dashboard.
6. Calcula a saída do PID e define a velocidade de cada motor com base na velocidade definida para o robô para o segmento da pista e/ou estado em que ele se encontra.
7. Calcula a velocidade (duty cycle do PWM) do robô com base na aceleração definida pelo usuário na dashboard.
8. Imprime na serial variáveis importantes do robô, permitindo a verificação do seu funcinamento.
