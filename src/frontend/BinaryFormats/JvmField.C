/* JVM Fields */
#include <featureTests.h>
#ifdef ROSE_ENABLE_BINARY_ANALYSIS
#include "sage3basic.h"

#include <Rose/Diagnostics.h>
#include "JvmClassFile.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace Rose::Diagnostics;
using namespace ByteOrder;

SgAsmJvmField::SgAsmJvmField(SgAsmJvmFieldTable* table)
{
  std::cout << "SgAsmJvmField::ctor() ...\n";

  set_parent(table);
  p_attribute_table = new SgAsmJvmAttributeTable(this);
  ROSE_ASSERT(get_attribute_table()->get_attributes());
  ROSE_ASSERT(get_attribute_table()->get_attributes()->get_parent());
}

SgAsmJvmField* SgAsmJvmField::parse(SgAsmJvmConstantPool* pool)
{
  ROSE_ASSERT(pool);
  std::cout << "SgAsmJvmField::parse() ...\n";

  Jvm::read_value(pool, p_access_flags);
  Jvm::read_value(pool, p_name_index);
  Jvm::read_value(pool, p_descriptor_index);

  p_attribute_table->parse(pool);
  dump(stdout, "", 0);

  std::cout << "SgAsmJvmField::parse() exit ...\n";

  return this;
}

void SgAsmJvmField::dump(FILE*f, const char* prefix, ssize_t idx) const
{
  fprintf(f, "SgAsmJvmField::%d:%d:%d\n", p_access_flags, p_name_index, p_descriptor_index);
}

SgAsmJvmFieldTable::SgAsmJvmFieldTable(SgAsmJvmClassFile* jcf)
{
  std::cout << "SgAsmJvmFieldTable::ctor() ...\n";

  set_parent(jcf);
  p_fields = new SgAsmJvmFieldList;
  p_fields->set_parent(this);
}

SgAsmJvmFieldTable* SgAsmJvmFieldTable::parse()
{
  uint16_t fields_count;

  std::cout << "SgAsmJvmFieldTable::parse() ...\n";

  auto jcf = dynamic_cast<SgAsmJvmClassFile*>(get_parent());
  ROSE_ASSERT(jcf && "JVM class_file is a nullptr");
  auto pool = jcf->get_constant_pool();
  ROSE_ASSERT(pool && "JVM constant_pool is a nullptr");

  auto fields = get_fields()->get_entries();

  Jvm::read_value(pool, fields_count);
  std::cout << "SgAsmJvmClassFile::fields_count " << fields_count << std::endl;

  for (int ii = 0; ii < fields_count; ii++) {
    auto method = new SgAsmJvmField(this);
    method->parse(pool);
    fields.push_back(method);
  }

  std::cout << "SgAsmJvmFieldTable::parse() exit ...\n";
  return this;
}

void SgAsmJvmFieldTable::dump(FILE*f, const char* prefix, ssize_t idx) const
{
  fprintf(f, "%s", prefix);
  for (auto field : get_fields()->get_entries()) {
    field->dump(stdout, "   ", idx++);
  }
}

#endif // ROSE_ENABLE_BINARY_ANALYSIS
