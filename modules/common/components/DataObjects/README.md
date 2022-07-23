# DataObjects
Essa pasta contém as classes utilizadas para lidar com o armazenamento de dados na memória Ram e flash do robô, sendo todos os parâmetros do robô criados com base nessas classes. As classes contidas nessa pasta são:

- [DataAbstract](./src/DataAbstract.hpp): Classe responsável por lidar com os parâmetros do robô, permitindo a leitura e escrita deles em tempo de execução, funcionando como um tipo genérico para diferentes tipos de parâmetros. 

- [DataMap](./src/DataMap.hpp): Classe responsável por lidar com os dados do mapeamento.

- [DataStorage](./src/DataStorage.hpp): Responsável por carregar parâmetros da flash ou salvá-los nela, através de arquivos.

- [DataManager](./src/DataManager.hpp): Responsável por gerenciar os parâmetros do robô através de uma lista de parâmetros que o permite facilmente acessá-los quando necessário.

- [IDataAbstract](./src/IDataAbstract.hpp): Interface utilizada como base para a classe DataMap e DataAbstract.