---
id: 2-5
title: Prefer Composition
category: Components
priority: MEDIUM
description: Prefer composition and context over prop drilling
---

## Problem

Passing data through many component layers (prop drilling) creates tight coupling, makes refactoring difficult, and clutters intermediate components with props they don't use. Prefer composition patterns and context for sharing data across component trees.

## Incorrect

```tsx
// ❌ WRONG: Prop drilling through multiple layers
function App() {
  const [user, setUser] = createSignal({ name: "John", role: "admin" });

  return <Layout user={user()} setUser={setUser} />;
}

function Layout(props) {
  return (
    <div>
      <Header user={props.user} setUser={props.setUser} />
      <Main user={props.user} setUser={props.setUser} />
      <Footer user={props.user} />
    </div>
  );
}

function Header(props) {
  return (
    <header>
      <Navigation user={props.user} />
      <UserMenu user={props.user} setUser={props.setUser} />
    </header>
  );
}

// And so on... props passed through many layers
```

## Correct

### Using Context

```tsx
import { createContext, useContext, createSignal, ParentComponent } from "solid-js";

// Create a typed context
interface UserContextValue {
  user: () => User;
  setUser: (user: User) => void;
  logout: () => void;
}

const UserContext = createContext<UserContextValue>();

// Provider component
const UserProvider: ParentComponent = (props) => {
  const [user, setUser] = createSignal<User>({ name: "John", role: "admin" });

  const value: UserContextValue = {
    user,
    setUser,
    logout: () => setUser(null)
  };

  return (
    <UserContext.Provider value={value}>
      {props.children}
    </UserContext.Provider>
  );
};

// Hook for consuming context
function useUser() {
  const context = useContext(UserContext);
  if (!context) {
    throw new Error("useUser must be used within UserProvider");
  }
  return context;
}

// ✅ CORRECT: Components access what they need directly
function App() {
  return (
    <UserProvider>
      <Layout />
    </UserProvider>
  );
}

function Layout() {
  return (
    <div>
      <Header />
      <Main />
      <Footer />
    </div>
  );
}

function UserMenu() {
  const { user, logout } = useUser();

  return (
    <div>
      <span>{user().name}</span>
      <button onClick={logout}>Logout</button>
    </div>
  );
}
```

### Using Composition with Slots

```tsx
// ✅ CORRECT: Compose at the top level
function App() {
  const [user, setUser] = createSignal({ name: "John" });

  return (
    <Layout
      header={<Header><UserMenu user={user()} onLogout={() => setUser(null)} /></Header>}
      footer={<Footer userName={user().name} />}
    >
      <Main user={user()} />
    </Layout>
  );
}

function Layout(props) {
  return (
    <div class="layout">
      <div class="header">{props.header}</div>
      <div class="main">{props.children}</div>
      <div class="footer">{props.footer}</div>
    </div>
  );
}
```

### Using Render Props

```tsx
// ✅ CORRECT: Render prop pattern for flexible composition
function DataFetcher<T>(props: {
  url: string;
  children: (data: T, loading: boolean) => JSX.Element;
}) {
  const [data] = createResource(() => props.url, fetchData);

  return props.children(data(), data.loading);
}

// Usage
<DataFetcher url="/api/users">
  {(users, loading) => (
    <Show when={!loading} fallback={<Spinner />}>
      <UserList users={users} />
    </Show>
  )}
</DataFetcher>
```

## When to Use Each Pattern

| Pattern | Use Case |
| ------- | -------- |
| Props | Direct parent-child relationships, 1-2 levels |
| Context | Global/shared state (auth, theme, i18n) |
| Composition/Slots | Layout components, structural flexibility |
| Render Props | Sharing logic with flexible rendering |

## Context Best Practices

```tsx
// 1. Create context with default value or undefined
const ThemeContext = createContext<ThemeContextValue>();

// 2. Provide a custom hook for type safety
function useTheme() {
  const ctx = useContext(ThemeContext);
  if (!ctx) throw new Error("useTheme requires ThemeProvider");
  return ctx;
}

// 3. Keep context values stable (use signals, not objects)
const value = {
  theme,      // Signal getter
  setTheme,   // Signal setter
  toggle      // Stable function
};

// 4. Split large contexts by concern
<AuthProvider>
  <ThemeProvider>
    <I18nProvider>
      <App />
    </I18nProvider>
  </ThemeProvider>
</AuthProvider>
```

## Why It Matters

1. **Decoupling**: Components don't need to know about intermediate layers.

2. **Maintenance**: Changing data flow doesn't require updating every component in the chain.

3. **Reusability**: Components without drilling dependencies are more reusable.

4. **Clarity**: Each component only receives props it actually uses.

## Related Rules

- [4-5: Use Context for Global State](4-5-use-context-for-global.md) - Detailed context patterns
- [2-4: Use children Helper](2-4-use-children-helper.md) - Working with composed children
