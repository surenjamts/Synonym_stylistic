import express from "express";
import fs from "fs";
import path from "path";
import { fileURLToPath } from "url";

type SynDict = Record<string, string[]>;
type ImproveResponse = {
  original: string;
  improved: string;
  repetitions: { word: string; count: number; replacements: string[] }[];
};

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

const app = express();
app.use(express.json({ limit: "2mb" }));

// serve index.html from /public
app.use(express.static(path.join(__dirname, "public")));

const SYN_PATH = path.join(__dirname, "synonyms.json");
const THRESHOLD = 3;

function loadSynonyms(): SynDict {
  const raw = fs.readFileSync(SYN_PATH, "utf8");
  const parsed = JSON.parse(raw) as SynDict;
  const out: SynDict = {};
  for (const [k, v] of Object.entries(parsed)) {
    out[k.toLowerCase()] = (v ?? []).map(String);
  }
  return out;
}

let SYNONYMS: SynDict = loadSynonyms();

const PUNCT_RE = /^[\s"'“‘(\[{]+|[\s"'”’)\]}.,;:!?]+$/g;

function normalizeToken(token: string): string {
  return token.toLowerCase().replace(PUNCT_RE, "");
}

function stripEnds(token: string): { lead: string; core: string; trail: string } {
  const m = token.match(/^([\s"'“‘(\[{]*)(.*?)([\s"'”’)\]}.,;:!?]*)$/);
  if (!m) return { lead: "", core: token, trail: "" };
  return { lead: m[1] ?? "", core: m[2] ?? token, trail: m[3] ?? "" };
}

function replacePreserveCase(originalCore: string, replacement: string): string {
  if (originalCore.length > 0 && originalCore[0] === originalCore[0].toUpperCase()) {
    return replacement.length ? replacement[0].toUpperCase() + replacement.slice(1) : replacement;
  }
  return replacement;
}

function improveText(text: string): ImproveResponse {
  const parts = text.split(/(\s+)/);
  const wordsOnly = parts.filter(p => p.trim() && !/^\s+$/.test(p));

  const freq = new Map<string, number>();
  for (const token of wordsOnly) {
    const { core } = stripEnds(token);
    const key = normalizeToken(core);
    if (!key) continue;
    freq.set(key, (freq.get(key) ?? 0) + 1);
  }

  const repetitions: ImproveResponse["repetitions"] = [];
  for (const [w, c] of freq.entries()) {
    if (c >= THRESHOLD && SYNONYMS[w]?.length) {
      repetitions.push({ word: w, count: c, replacements: SYNONYMS[w] });
    }
  }
  repetitions.sort((a, b) => b.count - a.count);

  const seen = new Map<string, number>();
  const improved = parts.map(p => {
    if (!p.trim() || /^\s+$/.test(p)) return p;

    const { lead, core, trail } = stripEnds(p);
    const key = normalizeToken(core);
    if (!key) return p;

    const total = freq.get(key) ?? 0;
    const syns = SYNONYMS[key];
    if (!syns || syns.length === 0 || total < THRESHOLD) return p;

    const k = (seen.get(key) ?? 0) + 1;
    seen.set(key, k);

    if (k === 1) return p; // keep first occurrence

    const idx = (k - 2) % syns.length; // 2nd occurrence -> 0
    const repl = replacePreserveCase(core, syns[idx]);
    return `${lead}${repl}${trail}`;
  }).join("");

  return { original: text, improved, repetitions };
}

app.post("/improve", (req, res) => {
  const text = typeof req.body?.text === "string" ? req.body.text : "";
  if (!text.trim()) return res.status(400).json({ error: "text is required" });
  res.json(improveText(text));
});

app.listen(3000, () => {
  console.log("http://localhost:3000");
});