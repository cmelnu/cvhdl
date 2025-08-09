Examples
========

This section showcases example C files and their generated VHDL output.

Example C file:
---------------

.. code-block:: c

   int add(int a, int b) {
       int sum = a + b;
       return sum;
   }

Generated VHDL
---------------

.. code-block:: vhdl

   entity add is
     port (
       clk : in std_logic;
       reset : in std_logic;
       a : in std_logic_vector(31 downto 0);
       b : in std_logic_vector(31 downto 0);
       result : out std_logic_vector(31 downto 0)
     );
   end entity;

   architecture behavioral of add is
   begin
     process(clk, reset)
     begin
       if reset = '1' then
         -- Reset logic
       elsif rising_edge(clk) then
         result <= a + b;
       end if;
     end process;
   end architecture;
