# services
Essa pasta contém os serviços (tasks) que estão rodando no robô, sendo que cada serviço possui sua função e está executando em paralelo com os outros serviços, através de multitasking com o FreeRTOS.

# Estrutura mínima:
## Cabeçalho
```cpp
#ifndef SERVICE_NAME_HPP
#define SERVICE_NAME_HPP

#include "thread.hpp"
class ServiceName : public Thread, 
{
public:
    ServiceName(std::string name, uint32_t stackDepth, UBaseType_t priority);
    void Run() override;

private:
};
#endif
```

## Implementação
```cpp
#include "ServiceName.hpp"

ServiceName::ServiceName(std::string name, uint32_t stackDepth, UBaseType_t priority) : Thread(name, stackDepth, priority)
{

}

void ServiceName::Run()
{

}