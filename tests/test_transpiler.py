import unittest

from compiler.holyc import TranslationError, translate


class TranslateTests(unittest.TestCase):
    def test_generates_native_entry_point(self):
        generated = translate("U0 Demo() {}\n\nDemo;\n", "Demo.HC")
        self.assertIn('#include "holyc.h"', generated)
        self.assertIn("Demo();", generated)
        self.assertIn("int main(int argc, char **argv)", generated)

    def test_requires_entry_point_at_end(self):
        with self.assertRaisesRegex(TranslationError, "final statement"):
            translate("U0 Main() {}\nMain;\nI64 later;\n", "Bad.HC")

    def test_rejects_direct_hardware_access(self):
        with self.assertRaisesRegex(TranslationError, "hardware operation"):
            translate("U0 Main() { OutU8(0x60, 1); }\nMain;", "Unsafe.HC")


if __name__ == "__main__":
    unittest.main()
