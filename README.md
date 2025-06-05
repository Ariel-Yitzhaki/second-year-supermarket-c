# second-year-supermarket-c
My second‐year CS project is a console-based supermarket manager written in C that lets you add/view products (with barcodes, prices, expiry dates), manage customers (including “club members” who receive discounts), and build shopping carts that update inventory automatically.
All product data is stored in a compressed binary file (SuperMarket.bin) – the program packs each product’s fields into bit-level form before writing and unpacks them when reading.
Customer data is stored in Customers.txt so work persists between runs.
Code is organized into modules for products, customers/club members (using a simple vtable for inheritance), shopping carts, and file I/O helpers.
To build, open the .sln in Visual Studio (or compile all .c files with GCC), then run the generated executable to use the menu-driven interface.
