type SearchItem = Readonly<{
  title: string;
  description?: string;
  url: string;
}>;

function isSearchItem(value: unknown): value is SearchItem {
  if (typeof value !== "object" || value === null) return false;

  const v = value as Record<string, unknown>;
  return (
    typeof v.title === "string" &&
    typeof v.url === "string" &&
    (typeof v.description === "string" || v.description === undefined)
  );
}

export function toSearchItems(data: unknown): ReadonlyArray<SearchItem> {
  if (!Array.isArray(data)) return [];
  return data.filter(isSearchItem);
}