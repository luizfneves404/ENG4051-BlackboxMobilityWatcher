Ótimo! Esse cabeçalho `MadgwickAHRS.h` que você mostrou é da **versão original do algoritmo de Madgwick**, mantida por desenvolvedores da comunidade Arduino (ou baseada no repositório da x-io).

Essa versão **não expõe diretamente os valores dos quatérnions (`q0`, `q1`, etc.)**, ao contrário de outras variantes, como a de Paul Stoffregen. Então, para adaptar seu código de cálculo de aceleração sem gravidade, precisamos **obter os quatérnions de forma indireta** ou **modificar o código para armazenar os quatérnions internamente**.

---

## ✅ Solução prática: acessar os quatérnions

### Opção A – Adicionar getters ao `Madgwick` (mais limpo)

Abra o `MadgwickAHRS.h` e adicione os seguintes métodos públicos:

```cpp
float getQ0() { return q0; }
float getQ1() { return q1; }
float getQ2() { return q2; }
float getQ3() { return q3; }
```

Agora você poderá fazer no seu código:

```cpp
float q0 = filter.getQ0();
float q1 = filter.getQ1();
float q2 = filter.getQ2();
float q3 = filter.getQ3();
```

Isso resolve totalmente o problema de acesso aos quatérnions para remover a gravidade corretamente.

---

### Opção B – (menos ideal) copiar o filtro para dentro do seu projeto e torná-lo "amigável"

Se você não quer modificar bibliotecas instaladas, pode copiar `MadgwickAHRS.h/.cpp` para dentro do seu projeto como arquivos locais e aí editar livremente — mas o ideal é só adicionar os getters, como acima.

---

## ✅ Adaptação no seu código com os getters

Aqui está como fica a parte relevante da sua classe `IMUSpeed`, adaptada para essa versão do Madgwick:

```cpp
float q0 = filter.getQ0();
float q1 = filter.getQ1();
float q2 = filter.getQ2();
float q3 = filter.getQ3();

float gxg = 2 * (q1 * q3 - q0 * q2);
float gyg = 2 * (q0 * q1 + q2 * q3);
float gzg = q0 * q0 - q1 * q1 - q2 * q2 + q3 * q3;
```

Essa parte continua igual — a diferença foi só o **acesso aos quatérnions**, que a versão original da biblioteca não disponibiliza publicamente por padrão.

---

### 📌 Recapitulando

* ✅ A versão original do `MadgwickAHRS.h` funciona perfeitamente.
* ❗️ Ela **não expõe os quatérnions** diretamente → **solução: adicionar `getQ0()` etc.**
* 🔧 Depois disso, sua estimativa de aceleração sem gravidade funcionará normalmente.

---

Se quiser, posso te mandar um `.cpp` completo já com isso tudo embutido, incluindo uma cópia inline do filtro com os getters. Quer isso?
